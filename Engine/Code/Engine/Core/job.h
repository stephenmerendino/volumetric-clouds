#pragma once

#include "Engine/Core/Common.hpp"

#include <vector>
#include <tuple>
#include <utility>

class Signal;

//-----------------------------------------------------
// Job
enum JobType : unsigned int
{
    JOB_TYPE_GENERIC,
    JOB_TYPE_LOGGING,
    JOB_TYPE_MAIN,
    JOB_TYPE_RENDERING,
    NUM_JOB_TYPES
};

enum JobStage : unsigned int
{
    JOB_STAGE_CREATED,
    JOB_STAGE_DISPATCHED,
    JOB_STAGE_ENQUEUED,
    JOB_STAGE_RUNNING,
    JOB_STAGE_FINISHED
};

typedef void(*job_work_cb)(void*);

class Job
{
friend class JobConsumer;

public:
    JobType             m_type;
    job_work_cb         m_work_cb;
    void*               m_user_data;
    std::vector<Job*>   m_dependents;
    unsigned int        m_num_dependencies;
    JobStage            m_stage;
    unsigned int        m_ref_count;

public:
    Job(JobType type, job_work_cb work_cb, void* user_data);

    void on_finish();
    void on_dependency_finished();
    void depends_on(Job* dependency);

private:
    void run();
};

//-----------------------------------------------------
// Job Consumer
class JobConsumer
{
public:
    std::vector<JobType> types;

public:
    void            add_type(JobType type);
    void            consume_job();
    unsigned int    consume_for_ms(unsigned int ms);
    unsigned int    consume_all();
};

//-----------------------------------------------------
// Job System
void            job_system_init(int num_generic_threads_requested = -1);
void            job_system_shutdown();
void            job_system_set_type_signal(JobType type, Signal* signal);
void            job_system_main_step();
void            job_system_main_step_for_ms(unsigned int ms);

Job*            job_create(JobType type, job_work_cb work_cb, void* user_data);
void            job_dispatch(Job* job);
void            job_release(Job* job);
void            job_dispatch_and_release(Job* job);
void            job_run(JobType type, job_work_cb work_cb, void* user_data);
void            job_wait(Job* job);
void            job_wait_and_release(Job* job);

//-----------------------------------------------------
// Friendly parameter passing
template<typename WORK_CB, typename ...ARGS>
struct work_pass_data_t
{
    WORK_CB             work_cb; 
    std::tuple<ARGS...> args;

    work_pass_data_t(WORK_CB work_cb, ARGS ...args)
        :work_cb(work_cb)
        ,args(args...)
    {}
};

template<typename WORK_CB, typename TUPLE, size_t... INDICES>
void forward_job_arguments_with_indices(WORK_CB work_cb, TUPLE& args, std::integer_sequence<size_t, INDICES...>)
{
    UNUSED(args); // to prevent warnings in case the user isn't passing args at all
    work_cb(std::get<INDICES>(args)...);
}

template<typename WORK_CB, typename ...ARGS>
void forward_arguments_job(void* ptr)
{
    work_pass_data_t<WORK_CB, ARGS...>* args = (work_pass_data_t<WORK_CB, ARGS...>*)ptr;
    forward_job_arguments_with_indices(args->work_cb, args->args, std::make_index_sequence<sizeof...(ARGS)>());
    delete args;
}

template<typename WORK_CB, typename ...ARGS>
Job* job_create(JobType type, WORK_CB work_cb, ARGS... args)
{
    work_pass_data_t<WORK_CB, ARGS...>* pass = new work_pass_data_t<WORK_CB, ARGS...>(work_cb, args...); 
    return job_create(type, forward_arguments_job<WORK_CB, ARGS...>, (void*)pass);
}

template<typename WORK_CB, typename ...ARGS>
void job_run(JobType type, WORK_CB work_cb, ARGS... args)
{
    Job* job = job_create(type, work_cb, args...);
    job_dispatch_and_release(job);
}

template <typename ...ARGS>
void job_dispatch(Job* job, ARGS... args)
{
    job_dispatch(job);
    job_dispatch(args...);
}

template <typename ...ARGS>
void job_dispatch_and_release(Job* job, ARGS... args)
{
    job_dispatch_and_release(job);
    job_dispatch_and_release(args...);
}