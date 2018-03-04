#include "Engine/Core/log.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/event.h"
#include "Engine/Core/Console.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/job.h"
#include "Engine/Thread/thread.h"
#include "Engine/Thread/thread_safe_queue.h"
#include "Engine/Thread/signal.h"
#include "Engine/Profile/mem_tracker.h"
#include "Engine/Profile/callstack.h"
#include "Engine/Profile/profiler.h"
#include "Engine/Config/build_config.h"

#include <string.h>
#include <stdio.h>
#include <map>
#include <ctime>
#include <set>

static CriticalSection* s_lock;

struct log_message_t
{
    const char* tag         = nullptr;
    const char* message     = nullptr;
    Callstack* callstack    = nullptr;
    tm timestamp;
};


static const char*                      s_log_directory;
static const char*                      s_log_filename;
static FILE*                            s_log_file              = nullptr;
static tm                               s_startup_time;
static thread_handle_t                  s_log_thread            = nullptr;
static Signal                           s_logger_signal;
static ThreadSafeQueue<log_message_t>   s_messages;
static bool                             s_logger_running        = false;
static bool                             s_flush_file_pending    = false;
static Event<log_message_t&>            s_log_event;
static std::map<std::string, Rgba>      s_tag_colors;

static bool                             s_is_whitelist_mode     = false;
static std::set<std::string>            s_tags;

#define DEFAULT_TAG     "default"
#define ERROR_TAG       "error"
#define WARNING_TAG     "warning"
#define SUCCESS_TAG     "success"

//------------------------------------------------------------
// Internal
static bool is_tag_present(const char* tag)
{
    PROFILE_SCOPE_FUNCTION();
    std::set<std::string>::iterator found = s_tags.find(tag);
    if(found == s_tags.end()){
        return false;
    }

    return true;
}

static bool filter_message(const log_message_t& message)
{
    SCOPE_LOCK(s_lock);
    PROFILE_SCOPE_FUNCTION();

    if(s_is_whitelist_mode && !is_tag_present(message.tag)){
        return true;
    }

    if(!s_is_whitelist_mode && is_tag_present(message.tag)){
        return true;
    }

    return false;
}

static Rgba get_tag_color(const char* tag)
{
    std::map<std::string, Rgba>::iterator found = s_tag_colors.find(tag);
    if(found != s_tag_colors.end()){
        return found->second;
    }

    return Rgba::WHITE;
}

static void print_callstack_to_file(FILE* file, Callstack* cs)
{
    PROFILE_SCOPE_FUNCTION();
    callstack_line_t lines[256];
    unsigned int num_lines = callstack_get_lines(lines, 256, cs);
    for(unsigned int line = 0; line < num_lines; line++){
        fprintf(file, "   %s(%u): %s\n", lines[line].filename, lines[line].line, lines[line].function_name);
    }
    fprintf(file, "\n");
}

static void print_callstack_to_dev_console(Callstack* cs, const Rgba& tag_color)
{
    PROFILE_SCOPE_FUNCTION();
    callstack_line_t lines[256];
    unsigned int num_lines = callstack_get_lines(lines, 256, cs);
    for(unsigned int line = 0; line < num_lines; line++){
        console_printf(tag_color, "   %s(%u): %s", lines[line].filename, lines[line].line, lines[line].function_name);
    }
    console_printf(tag_color, "");
}

static void print_callstack_to_debugger(Callstack* cs)
{
    PROFILE_SCOPE_FUNCTION();
    callstack_line_t lines[256];
    unsigned int num_lines = callstack_get_lines(lines, 256, cs);
    for(unsigned int line = 0; line < num_lines; line++){
        DebuggerPrintf("   %s(%u): %s\n", lines[line].filename, lines[line].line, lines[line].function_name);
    }
    DebuggerPrintf("\n");
}

// Copies current log file to a copy
// The last parameter is std::string because it comes from the console and I didn't want to copy the contents to a char*
static void log_copy_job(const char* log_directory, const char* log_filename, const std::string& copy_filename)
{
    // close the current log file
    fclose(s_log_file);

    std::string cur_full_path = Stringf("%s%s", log_directory, log_filename);
    std::string copy_full_path = Stringf("%s%s", log_directory, copy_filename.c_str());

    // copy the file to the new filename
    std::vector<unsigned char> buffer;
    LoadBinaryFileToBuffer(cur_full_path, buffer);
    SaveBufferToBinaryFile(copy_full_path, buffer);

    // reopen the file
    errno_t err = fopen_s(&s_log_file, cur_full_path.c_str(), "a+");
    if(err != 0){
        DebuggerPrintf("ERROR: Failed to open file [%s] for logging system\n", log_filename);
        exit(EXIT_FAILURE);
    }
}

static void log_message_job(log_message_t& message)
{
    PROFILE_SCOPE_FUNCTION();

    if(!filter_message(message)){
        s_log_event.trigger(message);
    }

    SAFE_DELETE(message.tag);
    SAFE_DELETE(message.message);
    destroy_callstack(message.callstack);
}

static void log_flush_job(FILE* log_file)
{
    fflush(log_file);
}

static void print_log_to_file(void* user_arg, log_message_t& message)
{
    PROFILE_SCOPE_FUNCTION();

    if(strlen(message.message) == 0){
        fprintf(s_log_file, "\n");
        return;
    }

    // build time stamp
	char time_string[25];
	strftime(time_string, 25, "%D %H:%M:%S", &message.timestamp);

    fprintf(s_log_file, "[%s][%s] %s\n", message.tag, time_string, message.message);

    if(nullptr != message.callstack){
        print_callstack_to_file(s_log_file, message.callstack);
    }
}

static void print_log_to_debugger(void* user_arg, log_message_t& message)
{
    PROFILE_SCOPE_FUNCTION();
    DebuggerPrintf("%s\n", message.message);

    if(message.callstack){
        print_callstack_to_debugger(message.callstack);
    }
}

static void print_log_to_dev_console(void* user_arg, log_message_t& message)
{
    PROFILE_SCOPE_FUNCTION();
    Rgba tag_color = get_tag_color(message.tag);
    console_printf(tag_color, "%s", message.message);

    if(nullptr != message.callstack){
        print_callstack_to_dev_console(message.callstack, tag_color);
    }
}

static void open_or_create_log_file(FILE** file, const char* directory, const char* filename)
{
    BOOL created = CreateDirectoryA(directory, NULL);
    if(created == 0){
        if(GetLastError() != ERROR_ALREADY_EXISTS){
            DebuggerPrintf("ERROR: Failed to create directory structure [%s] for logging system\n", directory);
            exit(EXIT_FAILURE);
        }
    }

    std::string full_path = Stringf("%s%s", directory, filename);

    errno_t err = fopen_s(file, full_path.c_str(), "w+");
    if(err != 0){
        DebuggerPrintf("ERROR: Failed to open file [%s] for logging system\n", filename);
        exit(EXIT_FAILURE);
    }
}

static void main_logging_thread()
{
    open_or_create_log_file(&s_log_file, s_log_directory, s_log_filename);

    s_logger_running = true;

    thread_set_name("Log");

    s_log_event.subscribe(nullptr, print_log_to_file);
    s_log_event.subscribe(nullptr, print_log_to_debugger);
    s_log_event.subscribe(nullptr, print_log_to_dev_console);

    JobConsumer log_consumer;
    log_consumer.add_type(JOB_TYPE_LOGGING);

    job_system_set_type_signal(JOB_TYPE_LOGGING, &s_logger_signal);

    while(s_logger_running){
        s_logger_signal.wait();
        log_consumer.consume_all();
    }

    fclose(s_log_file);
}

static void purge_old_logs()
{
    std::vector<WIN32_FIND_DATAA> files;

    WIN32_FIND_DATAA file_data;
    HANDLE fh = FindFirstFileA(COMBINE(LOG_FILE_DIRECTORY, LOG_PURGE_SEARCH_STRING), &file_data);

    if(fh == INVALID_HANDLE_VALUE){
        return;
    }

    files.push_back(file_data);
    while(FindNextFileA(fh, &file_data)){
        files.push_back(file_data);
    }

    // sort newest to oldest
	std::sort(files.begin(), files.end(), [](const WIN32_FIND_DATAA a, const WIN32_FIND_DATAA b) -> bool{
        return CompareFileTime(&a.ftCreationTime, &b.ftCreationTime) > 0;
	});

    while(files.size() > LOG_FILE_HISTORY){
        WIN32_FIND_DATAA f = files.back();

        // build the full file path
        char full_file_path[50];
        sprintf_s(full_file_path, "%s/%s", LOG_FILE_DIRECTORY, f.cFileName);

        // delete the actual file
        DeleteFileA(full_file_path);

        // remove from the list
        files.pop_back();
    }
}

static void init_tag_colors()
{
    log_set_console_tag_color(DEFAULT_TAG, Rgba::WHITE);
    log_set_console_tag_color(ERROR_TAG, Rgba::RED);
    log_set_console_tag_color(WARNING_TAG, Rgba::YELLOW);
    log_set_console_tag_color(SUCCESS_TAG, Rgba::GREEN);
}

static void create_unique_log_filename()
{
	char time_string[25];
	strftime(time_string, 25, LOG_DATE_FORMAT, &s_startup_time);

    int random_stamp = rand() * rand();

    s_log_filename = new char[50];
    sprintf_s((char* const)s_log_filename, 50, LOG_TIMESTAMP_FORMAT, time_string, random_stamp);
}

//------------------------------------------------------------
// Public API
void log_init(const char* log_directory)
{
	std::time_t raw_time = std::time(nullptr);
	localtime_s(&s_startup_time, &raw_time);

    s_lock = mem_construct_untracked_object<CriticalSection>();

    init_tag_colors();

    s_log_directory = log_directory;
    create_unique_log_filename();

    s_log_thread = thread_create(main_logging_thread);
}

void log_shutdown()
{
    if(!s_logger_running){
        return;
    }

    if(nullptr != s_log_thread){
        s_logger_running = false;
        s_logger_signal.signal_all();
        thread_join(s_log_thread);
    }

    delete[] s_log_filename;

    mem_destroy_untracked_object(s_lock);

    s_tags.clear();

    purge_old_logs();
}

void log_flush()
{
    if(!s_logger_running){
        return;
    }

    job_run(JOB_TYPE_LOGGING, log_flush_job, s_log_file);
}

static void log_tagged_printf_valist(const char* tag, const char* format, va_list arg_list, bool with_callstack)
{
    // build message
	char* message_text = new char[MAX_MESSAGE_SIZE];
	vsnprintf_s(message_text, MAX_MESSAGE_SIZE, _TRUNCATE, format, arg_list);	
	va_end(arg_list);
	message_text[MAX_MESSAGE_SIZE - 1] = '\0';

    // copy tag
    size_t tag_size = strlen(tag);
    char* tag_copy = new char[tag_size + 1];
    memcpy(tag_copy, tag, tag_size);
    tag_copy[tag_size] = '\0';

    // build data needed for job
    log_message_t message;
    message.tag = tag_copy;
    message.message = message_text;

	// get timestamp
	std::time_t rawTime = std::time(nullptr);
	localtime_s(&message.timestamp, &rawTime);

    if(with_callstack){
        message.callstack = create_callstack(2);
    }

    // submit job
    job_run(JOB_TYPE_LOGGING, log_message_job, message);
} 

void log_printf(const char* format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);

    log_tagged_printf_valist(DEFAULT_TAG, format, arg_list, false);
}

void log_tagged_printf(const char* tag, const char* format, ...) 
{
	va_list arg_list;
	va_start(arg_list, format);

    log_tagged_printf_valist(tag, format, arg_list, false);
}

void log_warningf(const char* format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);

    log_tagged_printf_valist(WARNING_TAG, format, arg_list, false);
}

void log_errorf(const char* format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);

    log_tagged_printf_valist(ERROR_TAG, format, arg_list, false);
    log_flush();
    DIE("Fatal Error Encountered");
}

void log_printf_with_callstack(const char* format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);

    log_tagged_printf_valist(DEFAULT_TAG, format, arg_list, true);
}

void log_tagged_printf_with_callstack(const char* tag, const char* format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);

    log_tagged_printf_valist(tag, format, arg_list, true);
}

void log_disable_tag(const char* tag)
{
    SCOPE_LOCK(s_lock);
    s_is_whitelist_mode = false;
    s_tags.insert(tag);
}

void log_enable_tag(const char* tag)
{
    SCOPE_LOCK(s_lock);
    s_is_whitelist_mode = true;
    s_tags.insert(tag);
}

void log_disable_all_tags()
{
    SCOPE_LOCK(s_lock);
    s_tags.clear();
    s_is_whitelist_mode = true;
}

void log_enable_all_tags()
{
    SCOPE_LOCK(s_lock);
    s_tags.clear();
    s_is_whitelist_mode = false;
}

void log_set_console_tag_color(const char* tag, const Rgba& color)
{
    s_tag_colors[tag] = color;
}

COMMAND(log_disable_tag, "[string:tag_name] Disables a tag for logging")
{
    log_disable_tag(args.next_string_arg().c_str());
}

COMMAND(log_enable_tag, "[string:tag_name] Enable a tag for logging")
{
    log_enable_tag(args.next_string_arg().c_str());
}

COMMAND(log_disable_all_tags, "Disables all tag for logging")
{
    log_disable_all_tags();
}

COMMAND(log_enable_all_tags, "Enables all tag for logging")
{
    log_enable_all_tags();
}

COMMAND(log_tagged, "[string:tagname string:message] Log a message with a specific tag")
{
    std::string tag = args.next_string_arg();
    std::string message = args.next_string_arg();
    log_tagged_printf(tag.c_str(), "%s", message.c_str());
}

COMMAND(log_copy, "[string:new_filename] Copies the current log to a new log file")
{
    job_run(JOB_TYPE_LOGGING, log_copy_job, s_log_directory, s_log_filename, args.next_string_arg());
}