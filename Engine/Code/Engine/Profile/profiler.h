#pragma once

#include "Engine/Config/build_config.h"
#include "Engine/Profile/auto_profile_scope.h"
#include "Engine/Profile/auto_profile_log_scope.h"
#include "Engine/Profile/thread_profile.h"

#include <stdint.h>
#include <memory>
#include <vector>

uint64_t                            get_current_perf_counter();
double                              perf_counter_to_seconds(uint64_t counter);

void                                profiler_init();
void                                profiler_shutdown();
void                                profiler_set_thread_name(const thread_id_t& id, const char* name);

void                                profiler_push(const char* tag);
void                                profiler_pop();
void                                profiler_track_alloc(size_t byte_size);
void                                profiler_track_free(size_t byte_size);

void                                profiler_pause_all();
void                                profiler_pause_thread(const thread_id_t& id);
void                                profiler_resume_all();
void                                profiler_resume_thread(const thread_id_t& id);
void                                profiler_step_all();
void                                profiler_step_thread(const thread_id_t& id);

std::shared_ptr<profiler_node_t>    profiler_get_prev_frame();
std::shared_ptr<profiler_node_t>    profiler_get_prev_frame(const char* root_tag);
std::shared_ptr<profiler_node_t>    profiler_get_prev_frame_for_thread(const thread_id_t& id);
std::shared_ptr<profiler_node_t>    profiler_get_prev_frame_for_thread(const thread_id_t& id, const char* root_tag);

void                                profiler_tree_report_last_frame_all();
void                                profiler_tree_report_last_frame_thread(const thread_id_t& id);

void                                profiler_flat_report_last_frame_all();
void                                profiler_flat_report_last_frame_thread(const thread_id_t& id);

std::vector<ThreadProfile*>         profiler_get_all_threads_snapshot();