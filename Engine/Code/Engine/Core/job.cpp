#include "Engine/Core/job.h"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/log.h"
#include "Engine/Core/Time.hpp"
#include "Engine/Thread/thread.h"
#include "Engine/Thread/signal.h"
#include "Engine/Thread/thread_safe_queue.h"
#include "Engine/Thread/atomic.h"
#include "Engine/Profile/profiler.h"
#include "Engine/Memory/thread_safe_block_allocator.h"
#include <thread>

//-----------------------------------------------------
// Internal helpers

static JobConsumer                  s_main_thread_consumer;
static ThreadSafeBlockAllocator*    s_job_allocator;

static unsigned int calculate_num_generic_threads_to_create(int num_generic_threads_requested)
{
    int num_generic_threads_to_create = 0;
    if(num_generic_threads_requested > 0){
        // if user passes in a positive number of generic threads, make that many 
        num_generic_threads_to_create = num_generic_threads_requested;
    }else{
        // else, subtract the number from core count and make that many
        int core_count = (int)std::thread::hardware_concurrency();
        num_generic_threads_to_create = core_count + num_generic_threads_requested;
    }
    num_generic_threads_to_create = max(1, num_generic_threads_to_create);
    return num_generic_threads_to_create;
}

class JobSystem
{
public:
    unsigned int            m_num_generic_threads;
    thread_handle_t*        m_generic_threads;
    JobConsumer             m_generic_consumer;

    unsigned int            m_num_queues;
    ThreadSafeQueue<Job*>*  m_queues;
    Signal**                m_signals;

    bool                    m_is_running;

public:
    JobSystem(unsigned int num_generic_threads_to_create);
    ~JobSystem();

    void init();
    void shutdown();
};

static JobSystem* g_job_system = nullptr;

JobSystem::JobSystem(unsigned int num_generic_threads_to_create)
    :m_num_generic_threads(num_generic_threads_to_create)
    ,m_generic_threads(nullptr)
    ,m_num_queues(NUM_JOB_TYPES)
    ,m_queues(nullptr)
    ,m_signals(nullptr)
    ,m_is_running(false)
{
    m_generic_threads       = new thread_handle_t[num_generic_threads_to_create];
    m_queues                = new ThreadSafeQueue<Job*>[NUM_JOB_TYPES];
    m_signals               = new Signal*[NUM_JOB_TYPES];

    memset(m_signals, 0, sizeof(Signal*) * NUM_JOB_TYPES);
    memset(m_generic_threads, 0, sizeof(thread_handle_t) * num_generic_threads_to_create);

    m_signals[JOB_TYPE_GENERIC]  = new Signal();
    m_generic_consumer.add_type(JOB_TYPE_GENERIC);
}

JobSystem::~JobSystem()
{
    shutdown();
}

static void generic_job_consumer_thread(int worker_id)
{
    std::string thread_name = Stringf("Generic Job Worker #%i", worker_id);
    thread_set_name(thread_name.c_str()); 

    while(g_job_system->m_is_running){
        g_job_system->m_signals[JOB_TYPE_GENERIC]->wait();
        g_job_system->m_generic_consumer.consume_all();
    }

    g_job_system->m_generic_consumer.consume_all();
}

void JobSystem::init()
{
    m_is_running = true;

    // actually spin up the generic worker threads
    for(unsigned int i = 0; i < m_num_generic_threads; ++i){
        m_generic_threads[i] = thread_create(generic_job_consumer_thread, i);
    }
}

void JobSystem::shutdown()
{
    if(m_is_running){
        m_is_running = false;

        for(unsigned int i = 0; i < NUM_JOB_TYPES; ++i){
            Signal* signal = m_signals[i];
            if(nullptr != signal){
                signal->signal_all();
            }
        }

        for(unsigned int i = 0; i < m_num_generic_threads; ++i){
            thread_join(m_generic_threads[i]);
        }
    }

    SAFE_DELETE(m_generic_threads);
    SAFE_DELETE(m_signals[JOB_TYPE_GENERIC]);
    SAFE_DELETE(m_signals);
}

//-----------------------------------------------------
// Job
Job::Job(JobType type, job_work_cb work_cb, void* user_data)
    :m_type(type)
    ,m_work_cb(work_cb)
    ,m_user_data(user_data)
    ,m_num_dependencies(1)
    ,m_stage(JOB_STAGE_CREATED)
    ,m_ref_count(1)
{
}

void Job::on_finish()
{
    for(Job* dependent : m_dependents){
        dependent->on_dependency_finished();
        job_release(this);
    }
}

void Job::on_dependency_finished()
{
    job_dispatch(this);
}

void Job::depends_on(Job* dependency)
{
    atomic_incr(&m_num_dependencies);
    atomic_incr(&dependency->m_ref_count);
    dependency->m_dependents.push_back(this);
}

void Job::run()
{
    m_stage = JOB_STAGE_RUNNING;
    m_work_cb(m_user_data);

    m_stage = JOB_STAGE_FINISHED;
    on_finish();

    job_release(this);
}

//-----------------------------------------------------
// Job Consumer
void JobConsumer::add_type(JobType type)
{
    types.push_back(type);
}

void JobConsumer::consume_job()
{
    Job* job = nullptr;
    for(JobType type : types){
        ThreadSafeQueue<Job*>& queue = g_job_system->m_queues[type];
        if(queue.pop(&job)){
            job->run();
            return;
        }
    }
}

unsigned int JobConsumer::consume_for_ms(unsigned int ms)
{
    double start = get_current_time_seconds();

    unsigned int num_processed_jobs = 0;
    Job* job = nullptr;

    for(JobType type : types){
        ThreadSafeQueue<Job*>& queue = g_job_system->m_queues[type];
        while(queue.pop(&job)){
            job->run();
            ++num_processed_jobs;

            double elapsed_seconds = get_current_time_seconds() - start;
            unsigned int elapsed_ms = (unsigned int)(elapsed_seconds * 1000.0f);
            if(elapsed_ms >= ms){
                return num_processed_jobs;
            }
        }
    }

    return num_processed_jobs;
}

unsigned int JobConsumer::consume_all()
{
    return consume_for_ms(UINT_MAX);
}

//-----------------------------------------------------
// Public API

void job_system_init(int num_generic_threads_requested)
{
    s_job_allocator = new ThreadSafeBlockAllocator(sizeof(Job));

    unsigned int num_generic_threads_to_create = calculate_num_generic_threads_to_create(num_generic_threads_requested);
    g_job_system = new JobSystem(num_generic_threads_to_create);
    g_job_system->init();

    s_main_thread_consumer.add_type(JOB_TYPE_MAIN);
    s_main_thread_consumer.add_type(JOB_TYPE_RENDERING);
}

void job_system_shutdown()
{
    SAFE_DELETE(g_job_system);
    SAFE_DELETE(s_job_allocator);
}

void job_system_set_type_signal(JobType type, Signal* signal)
{
    if(nullptr != g_job_system){
        g_job_system->m_signals[type] = signal;
    }
}

void job_system_main_step()
{
    s_main_thread_consumer.consume_all();
}

void job_system_main_step_for_ms(unsigned int ms)
{
    s_main_thread_consumer.consume_for_ms(ms);
}

Job* job_create(JobType type, job_work_cb work_cb, void* user_data)
{
    return s_job_allocator->create<Job>(type, work_cb, user_data);
}

void job_dispatch(Job* job)
{
    atomic_incr(&job->m_ref_count);

    job->m_stage = JOB_STAGE_DISPATCHED;
    unsigned int num_dependencies_left = atomic_decr(&job->m_num_dependencies);
    if(num_dependencies_left != 0){
        return;
    }

    job->m_stage = JOB_STAGE_ENQUEUED;
    g_job_system->m_queues[job->m_type].push(job);

    Signal* signal = g_job_system->m_signals[job->m_type];
    if(nullptr != signal){
        signal->signal_all();
    }
}

void job_release(Job* job)
{
    unsigned int ref_count = atomic_decr(&job->m_ref_count);
    if(ref_count == 0){
        s_job_allocator->free(job);
    }
}

void job_dispatch_and_release(Job* job)
{
    job_dispatch(job);
    job_release(job);
}

void job_run(JobType type, job_work_cb work_cb, void* user_data)
{
    Job* job = job_create(type, work_cb, user_data);
    job_dispatch_and_release(job);
}

void job_wait(Job* job)
{
    while(job->m_stage != JOB_STAGE_FINISHED){
        thread_yield();
    }
}

void job_wait_and_release(Job* job)
{
    job_wait(job);
    job_release(job);
}