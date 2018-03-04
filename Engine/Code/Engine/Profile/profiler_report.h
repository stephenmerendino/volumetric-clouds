#pragma once

#include "Engine/Profile/profiler.h"
#include <memory>
#include <vector>
#include <map>

struct report_node_t
{
    ~report_node_t();

    report_node_t*                          parent                  = nullptr;
    std::map<std::string, report_node_t*>   children_by_tag;
    const char*                             tag_name                = nullptr;
    int                                     calls                   = 0;
    float                                   total_percent           = 0.0f;
    double                                  total_time_seconds      = 0.0;
    float                                   self_percent            = 0.0f;
    double                                  self_time_seconds       = 0.0;
    double                                  child_time_seconds      = 0.0;
    double                                  avg_total_time_seconds          = 0.0;
    double                                  avg_self_time_seconds           = 0.0;
};

class ProfilerReport
{
public:
    ThreadProfile*                      m_thread_profile;
    report_node_t*                      m_frame_root;
    std::vector<report_node_t*>         m_flat_view;

public:
    ProfilerReport(ThreadProfile& thread_profile);
    ~ProfilerReport();

    void create_tree_view();
    void create_flat_view();
    void create_tree_view_for_frame(int frame_number);
    void create_flat_view_for_frame(int frame_number);

    void sort_by_total_time();
    void sort_by_self_time();

    void log();
    void store(std::vector<std::string>& storage);

private:
    void add_node_to_tree_report(profiler_node_t* node, report_node_t* parent = nullptr);
    void add_node_to_flat_report(profiler_node_t* node);
    void update_report_node_stats(report_node_t* report_node, profiler_node_t* profiler_node);
    report_node_t* find_or_create_flat_report_node(profiler_node_t* node);
    report_node_t* find_or_create_tree_report_node(profiler_node_t* node, report_node_t* parent);
};