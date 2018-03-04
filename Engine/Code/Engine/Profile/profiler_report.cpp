#include "Engine/Profile/profiler_Report.h"
#include "Engine/Core/log.h"
#include "Engine/Core/StringUtils.hpp"
#include <algorithm>

static double calc_elapsed_time(profiler_node_t* node)
{
    uint64_t elapsed = node->end_counter - node->start_counter;
    return perf_counter_to_seconds(elapsed);
}

static double calc_tree_total_time(profiler_node_t* node)
{
    if(nullptr == node){
        return 0.0;
    }

    profiler_node_t* cursor = node->first_child;

    if(nullptr == cursor){
        return calc_elapsed_time(node);
    }

    double sum_of_children = 0.0;

    do{
        sum_of_children += calc_tree_total_time(cursor);
        cursor = cursor->next_sibling;
    }while(cursor != node->first_child);

    return sum_of_children;
}

static double calc_children_total_time(profiler_node_t* node)
{
    if(nullptr == node){
        return 0.0;
    }

    profiler_node_t* cursor = node->first_child;

    if(nullptr == cursor){
        return 0.0;
    }

    double sum_of_children = 0.0;

    do{
        sum_of_children += calc_tree_total_time(cursor);
        cursor = cursor->next_sibling;
    }while(cursor != node->first_child);

    return sum_of_children;
}

static void log_tree_view(const report_node_t* root, unsigned int indent)
{
    if(nullptr == root){
        return;
    }

    char total_time_string[20];
    char self_time_string[20];
    char avg_total_time_string[20];
    char avg_self_time_string[20];

    pretty_print_time(total_time_string, 20, root->total_time_seconds);
    pretty_print_time(self_time_string, 20, root->self_time_seconds);
    pretty_print_time(avg_total_time_string, 20, root->avg_total_time_seconds);
    pretty_print_time(avg_self_time_string, 20, root->avg_self_time_seconds);

    // http://imgur.com/NxUhHIC
    log_tagged_printf("profiler", "%*s%-*s%*i%*.4f %%%*s%*.3f %%%*s%*s%*s", indent, " ", 70 - indent, root->tag_name, 
                                                                                         5,           root->calls, 
                                                                                         16,          root->total_percent, 
                                                                                         15,          total_time_string, 
                                                                                         12,          root->self_percent, 
                                                                                         15,          self_time_string,
                                                                                         16,          avg_total_time_string,
                                                                                         16,          avg_self_time_string);

    for(std::pair<std::string, report_node_t*> pair : root->children_by_tag){
        log_tree_view(pair.second, indent + 2);
    }
}

static void log_flat_view(const report_node_t* node)
{
    if(nullptr == node){
        return;
    }

    char total_time_string[20];
    char self_time_string[20];
    char avg_total_time_string[20];
    char avg_self_time_string[20];

    pretty_print_time(total_time_string, 20, node->total_time_seconds);
    pretty_print_time(self_time_string, 20, node->self_time_seconds);
    pretty_print_time(avg_total_time_string, 20, node->avg_total_time_seconds);
    pretty_print_time(avg_self_time_string, 20, node->avg_self_time_seconds);

    // http://imgur.com/NxUhHIC
    log_tagged_printf("profiler", "  %-*s%*i%*.4f %%%*s%*.3f %%%*s%*s%*s", 68, node->tag_name, 
                                                                           5,  node->calls, 
                                                                           16, node->total_percent, 
                                                                           15, total_time_string, 
                                                                           12, node->self_percent, 
                                                                           15, self_time_string,
                                                                           16, avg_total_time_string,
                                                                           16, avg_self_time_string);
}

static void store_tree_view(const report_node_t* root, int indent, std::vector<std::string>& storage)
{
    if(nullptr == root){
        return;
    }

    char total_time_string[20];
    char self_time_string[20];
    char avg_total_time_string[20];
    char avg_self_time_string[20];

    pretty_print_time(total_time_string, 20, root->total_time_seconds);
    pretty_print_time(self_time_string, 20, root->self_time_seconds);
    pretty_print_time(avg_total_time_string, 20, root->avg_total_time_seconds);
    pretty_print_time(avg_self_time_string, 20, root->avg_self_time_seconds);

    // http://imgur.com/NxUhHIC
    storage.push_back(Stringf("%*s%-*s%*i%*.4f %%%*s%*.3f %%%*s%*s%*s", indent, " ", 70 - indent, root->tag_name, 
                                                                                     5,           root->calls, 
                                                                                     16,          root->total_percent, 
                                                                                     15,          total_time_string, 
                                                                                     12,          root->self_percent, 
                                                                                     15,          self_time_string,
                                                                                     16,          avg_total_time_string,
                                                                                     16,          avg_self_time_string));

    for(std::pair<std::string, report_node_t*> pair : root->children_by_tag){
        store_tree_view(pair.second, indent + 2, storage);
    }
}

static void store_flat_view(const report_node_t* node, std::vector<std::string>& storage)
{
    if(nullptr == node){
        return;
    }

    char total_time_string[20];
    char self_time_string[20];
    char avg_total_time_string[20];
    char avg_self_time_string[20];

    pretty_print_time(total_time_string, 20, node->total_time_seconds);
    pretty_print_time(self_time_string, 20, node->self_time_seconds);
    pretty_print_time(avg_total_time_string, 20, node->avg_total_time_seconds);
    pretty_print_time(avg_self_time_string, 20, node->avg_self_time_seconds);

    // http://imgur.com/NxUhHIC
    storage.push_back(Stringf("  %-*s%*i%*.4f %%%*s%*.3f %%%*s%*s%*s", 68, node->tag_name, 
                                                                       5,  node->calls, 
                                                                       16, node->total_percent, 
                                                                       15, total_time_string, 
                                                                       12, node->self_percent, 
                                                                       15, self_time_string,
                                                                       16, avg_total_time_string,
                                                                       16, avg_self_time_string));
}

report_node_t::~report_node_t()
{
    for(std::pair<std::string, report_node_t*> p : children_by_tag){
        delete p.second;
    }
}

report_node_t* ProfilerReport::find_or_create_tree_report_node(profiler_node_t* node, report_node_t* parent)
{
    if(nullptr != parent){
        std::map<std::string, report_node_t*>::iterator found = parent->children_by_tag.find(node->tag);
        if(found != parent->children_by_tag.end()){
            return found->second;
        }
    }

    // create new node and add to map
    report_node_t* new_node = new report_node_t();
    new_node->tag_name = node->tag;
    new_node->parent = parent;
    if(nullptr != parent){
        parent->children_by_tag[node->tag] = new_node;
    }
    return new_node;
}

report_node_t* ProfilerReport::find_or_create_flat_report_node(profiler_node_t* node)
{
    // if already in vector, return it
    for(report_node_t* report_node : m_flat_view){
        if(0 == strcmp(node->tag, report_node->tag_name)){
            return report_node;
        }
    } 

    // create new node and add to vector
    report_node_t* new_node = new report_node_t();
    new_node->tag_name = node->tag;
    m_flat_view.push_back(new_node);
    return new_node;
}

ProfilerReport::ProfilerReport(ThreadProfile& thread_profile)
    :m_thread_profile(&thread_profile)
    ,m_frame_root(nullptr)
{
}

ProfilerReport::~ProfilerReport()
{
    if(!m_flat_view.empty()){
        for(report_node_t* t : m_flat_view){
            SAFE_DELETE(t);
        }
        m_frame_root = nullptr;
    }else{
        SAFE_DELETE(m_frame_root);
    }
}

void ProfilerReport::create_tree_view()
{
    std::shared_ptr<profiler_node_t> prev_frame = m_thread_profile->get_prev_frame();
    add_node_to_tree_report(prev_frame.get());
}

void ProfilerReport::create_flat_view()
{
    std::shared_ptr<profiler_node_t> prev_frame = m_thread_profile->get_prev_frame();
    add_node_to_flat_report(prev_frame.get());
}

void ProfilerReport::create_tree_view_for_frame(int frame_number)
{
    std::shared_ptr<profiler_node_t> prev_frame = m_thread_profile->m_saved_trees[frame_number];
    add_node_to_tree_report(prev_frame.get());
}

void ProfilerReport::create_flat_view_for_frame(int frame_number)
{
    std::shared_ptr<profiler_node_t> prev_frame = m_thread_profile->m_saved_trees[frame_number];
    add_node_to_flat_report(prev_frame.get());
}

void ProfilerReport::sort_by_total_time()
{
    if(m_flat_view.empty()){
        return;
    }

	std::sort(m_flat_view.begin(), m_flat_view.end(), [](const report_node_t* a, const report_node_t* b) -> bool{
        return a->total_time_seconds > b->total_time_seconds;
	});
}

void ProfilerReport::sort_by_self_time()
{
    if(m_flat_view.empty()){
        return;
    }

	std::sort(m_flat_view.begin(), m_flat_view.end(), [](const report_node_t* a, const report_node_t* b) -> bool{
        return a->self_time_seconds > b->self_time_seconds;
	});
}

void ProfilerReport::log()
{
    log_tagged_printf("profiler", "%s[id:%u]", nullptr == m_thread_profile->m_name ? "Unnamed" : m_thread_profile->m_name, m_thread_profile->m_id);

    if(nullptr == m_frame_root){
        log_tagged_printf("profiler", "  No Tracked Nodes");
        return;
    }

    log_tagged_printf("profiler", "  %-68s%*s%*s%*s%*s%*s%*s%*s", "TAG", 5, "CALLS", 18, "TOTAL%", 15, "TOTAL TIME", 14, "SELF%", 15, "SELF TIME", 16, "AVG TOTAL TIME", 16, "AVG SELF TIME");

    if(!m_flat_view.empty()){
        for(report_node_t* n : m_flat_view){
            log_flat_view(n);
        }
        m_flat_view.clear();
    }else{
        log_tree_view(m_frame_root, 2);
    }
}

void ProfilerReport::store(std::vector<std::string>& storage)
{
    if(nullptr == m_frame_root){
        storage.push_back("  No Tracked Nodes");
        return;
    }

    storage.push_back(Stringf("  %-68s%*s%*s%*s%*s%*s%*s%*s", "TAG", 5, "CALLS", 18, "TOTAL%", 15, "TOTAL TIME", 14, "SELF%", 15, "SELF TIME", 16, "AVG TOTAL TIME", 16, "AVG SELF TIME"));

    if(!m_flat_view.empty()){
        for(report_node_t* n : m_flat_view){
            store_flat_view(n, storage);
        }
    }else{
        store_tree_view(m_frame_root, 2, storage);
    }
}

void ProfilerReport::update_report_node_stats(report_node_t* report_node, profiler_node_t* profiler_node)
{
    // if root is null, this report node is now the root
    if(nullptr == m_frame_root){
        m_frame_root = report_node;
    }

    report_node->calls++;
    report_node->total_time_seconds += calc_elapsed_time(profiler_node);
    report_node->child_time_seconds += calc_children_total_time(profiler_node);
    report_node->self_time_seconds = (report_node->total_time_seconds - report_node->child_time_seconds);
    report_node->total_percent = (float)(report_node->total_time_seconds / m_frame_root->total_time_seconds) * 100.0f;
    report_node->self_percent = (float)(report_node->self_time_seconds / m_frame_root->total_time_seconds) * 100.0f;
    report_node->avg_total_time_seconds = report_node->total_time_seconds / (double)report_node->calls;
    report_node->avg_self_time_seconds = report_node->self_time_seconds / (double)report_node->calls;
}

void ProfilerReport::add_node_to_tree_report(profiler_node_t* node, report_node_t* parent)
{
    if(nullptr == node){
        return;
    }

    // find or create report_node_t based from parent
    report_node_t* report_node = find_or_create_tree_report_node(node, parent);
    update_report_node_stats(report_node, node);

    // check if we have any children
    profiler_node_t* cursor = node->first_child;
    if(nullptr == cursor){
        return;
    }

    // recurse the tree and add them
    do{
        add_node_to_tree_report(cursor, report_node);
        cursor = cursor->next_sibling;
    }while(cursor != node->first_child);
}

void ProfilerReport::add_node_to_flat_report(profiler_node_t* node)
{
    if(nullptr == node){
        return;
    }

    // find or create report_node_t based from parent
    report_node_t* report_node = find_or_create_flat_report_node(node);
    update_report_node_stats(report_node, node);

    // check if we have any children
    profiler_node_t* cursor = node->first_child;
    if(nullptr == cursor){
        return;
    }

    // recurse the tree and add them
    do{
        add_node_to_flat_report(cursor);
        cursor = cursor->next_sibling;
    }while(cursor != node->first_child);
}