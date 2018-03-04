#include "Engine/Profile/profiler_visualizer.h"
#include "Engine/Profile/mem_tracker.h"
#include "Engine/Profile/thread_profile.h"
#include "Engine/Profile/profiler.h"
#include "Engine/Profile/profiler_report.h"
#include "Engine/Engine.hpp"
#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/Renderer/QuadMeshes.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/log.h"
#include "Engine/Config/build_config.h"

COMMAND(profiler_show, "Show the profiler")
{
    profiler_show();
}

COMMAND(profiler_hide, "Show the profiler")
{
    profiler_hide();
}

COMMAND(toggle_profiler, "Show the profiler")
{
    profiler_toggle_visibility();
}

#define FONT_SCALE 0.020f

struct thread_visualizer_settings_t
{
    thread_id_t id;
    bool show_tree_view = true;
    bool is_tree_minimized = false;
    bool is_thread_minimized = false;
    bool force_frame = false;
    int frame_number_selected = -1;
    AABB2 expand_bounds;
};

static std::map<thread_id_t, thread_visualizer_settings_t*> s_thread_settings;
static bool s_profiler_shown = false; 

thread_visualizer_settings_t* find_or_create_thread_settings(const thread_id_t& id)
{
    std::map<thread_id_t, thread_visualizer_settings_t*>::iterator found = s_thread_settings.find(id);
    if(found != s_thread_settings.end()){
        return found->second;
    }

    thread_visualizer_settings_t* new_settings = new thread_visualizer_settings_t();
    new_settings->id = id;
    new_settings->show_tree_view = true;
    new_settings->is_tree_minimized = true;
    new_settings->is_thread_minimized = true;
    s_thread_settings[id] = new_settings;
    return new_settings;
}

bool is_tree_minimized(const thread_id_t& id)
{
    return find_or_create_thread_settings(id)->is_tree_minimized;
}

bool is_in_tree_view_mode(const thread_id_t& id)
{
    return find_or_create_thread_settings(id)->show_tree_view;
}

void toggle_minimize(const thread_id_t& id)
{
    thread_visualizer_settings_t* settings = find_or_create_thread_settings(id);
    settings->is_tree_minimized = !settings->is_tree_minimized;
}

void switch_view(const thread_id_t& id)
{
    thread_visualizer_settings_t* settings = find_or_create_thread_settings(id);
    settings->show_tree_view = !settings->show_tree_view;
}

void profiler_show()
{
    s_profiler_shown = true;
}

void profiler_hide()
{
    s_profiler_shown = false;
}

void profiler_toggle_visibility()
{
    if(s_profiler_shown){
        s_profiler_shown = false;
    }else{
        s_profiler_shown = true;
    }
}

#if defined(TRACK_MEMORY)
static void draw_memory_graph(const Vector2& top_left)
{
    AABB2 bounds;
    float height = get_engine_font().GetLineHeight(0.020f) * 7.f;
    float width = 1.0f;
    bounds.mins = Vector2(top_left.x + 0.4f, top_left.y - height);
    bounds.maxs = bounds.mins + Vector2(width, height);

    g_theRenderer->SetTexture(nullptr);
    g_theRenderer->DrawQuad2d(bounds, AABB2::ZERO_TO_ONE, Rgba(0,0,0,100));

    size_t current = mem_get_live_alloc_byte_size();

    size_t* frame_alloc_history = mem_get_frame_alloc_history();
    float bar_width = width / (float)MEMORY_TRACKER_FRAME_HISTORY;

    Vector2 bar_cursor = bounds.mins;

    MeshBuilder mb;
    for(int i = 0; i < MEMORY_TRACKER_FRAME_HISTORY; i++)
    {
        float scale = Clamp(((float)frame_alloc_history[i] / (float)current), 0.0f, 1.0f);

        AABB2 bar_bounds;
        bar_bounds.mins = bar_cursor;
        bar_bounds.maxs = bar_bounds.mins + Vector2(bar_width, height * scale);
        Meshes::build_quad_2d(mb, bar_bounds, AABB2::ZERO_TO_ONE, Rgba(0, 255, 0, 200), Rgba(0, 255, 0, 200));
        bar_cursor.x += bar_width;
    }

    g_theRenderer->draw_with_meshbuilder(mb);

    Vector2 cursor = bounds.maxs;
    cursor.x += 0.005f;

    char bytes_string[64];
    g_theRenderer->DrawText2d(cursor, 0.018f, Rgba::WHITE, bytes_to_string(bytes_string, 64, current), get_engine_font());

    cursor.y = bounds.mins.y + get_engine_font().GetLineHeight(0.020f);
    g_theRenderer->DrawText2d(cursor, 0.018f, Rgba::WHITE, "0 B", get_engine_font());

    cursor = bounds.mins;
    cursor.y -= 0.001f;
    g_theRenderer->DrawText2d(cursor, 0.018f, Rgba::WHITE, Stringf("%i", MEMORY_TRACKER_FRAME_HISTORY), get_engine_font());

    cursor = bounds.mins + Vector2(width - get_engine_font().GetTextWidth("0", 0.020f), 0.0f);
    cursor.y -= 0.001f;
    g_theRenderer->DrawText2d(cursor, 0.018f, Rgba::WHITE, "0", get_engine_font());
}
#endif

#if defined(PROFILED_BUILD)
static bool bar_bounds_clicked(const AABB2& bounds)
{
    g_theRenderer->SetTexture(nullptr);

    Vector2 mouse = g_theInputSystem->GetCursorClientAreaNormalized();
    mouse.x *= g_theRenderer->m_output->GetAspectRatio();
    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_LMB)){
        if(bounds.IsPointInside(mouse)){
            return true;
        }
    }

    return false;
}
#endif

#if defined(PROFILED_BUILD)
static bool bounds_clicked(Vector2& cursor, const char* text, const Rgba& bg_color = Rgba(0, 0, 0, 50))
{
    Vector2 mins = Vector2(cursor.x, cursor.y - get_engine_font().GetLineHeight(FONT_SCALE));
    Vector2 maxs = Vector2(cursor.x + get_engine_font().GetTextWidth(text, FONT_SCALE), cursor.y);
    AABB2 bounds(mins, maxs);
    g_theRenderer->SetTexture(nullptr);
    //g_theRenderer->DebugDrawBox2d(bounds, 0.001f, 0.0f, Rgba::PINK, Rgba::TRANSPARENT_WHITE);
    AABB2 box_bounds = bounds;
    box_bounds.Scale(1.1f);
    g_theRenderer->DrawQuad2d(box_bounds, AABB2::ZERO_TO_ONE, bg_color);

    Vector2 mouse = g_theInputSystem->GetCursorClientAreaNormalized();
    mouse.x *= g_theRenderer->m_output->GetAspectRatio();
    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_LMB)){
        if(bounds.IsPointInside(mouse)){
            return true;
        }
    }

    return false;
}
#endif

#if defined(PROFILED_BUILD)
static void draw_tree_tool(Vector2& cursor, ThreadProfile* profile)
{
    float original_x = cursor.x;

    thread_visualizer_settings_t* settings = find_or_create_thread_settings(profile->m_id);

    cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);

    if(bounds_clicked(cursor, "Tree")){
        settings->show_tree_view = true;
    }

    if(settings->show_tree_view){
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "Tree", get_engine_font());
    }else{
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::GREY, "Tree", get_engine_font());
    }


    cursor.x += get_engine_font().GetTextWidth("Tree", FONT_SCALE);
    g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, " / ", get_engine_font());

    cursor.x += get_engine_font().GetTextWidth(" / ", FONT_SCALE);

    if(bounds_clicked(cursor, "Flat")){
        settings->show_tree_view = false;
    }

    if(!settings->show_tree_view){
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "Flat ", get_engine_font());
    }else{
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::GREY, "Flat ", get_engine_font());
    }


    cursor.x += get_engine_font().GetTextWidth("Flat ", FONT_SCALE);
    if(is_tree_minimized(profile->m_id)){
        if(bounds_clicked(cursor, "[+]")){
            settings->is_tree_minimized = false;
        }
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "[+]", get_engine_font());
    }else{
        if(bounds_clicked(cursor, "[-]")){
            settings->is_tree_minimized = true;
        }
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "[-]", get_engine_font());
    }

    cursor.x = original_x;
}
#endif

#if defined(PROFILED_BUILD)
static void draw_thread_graph(Vector2& cursor, ThreadProfile* profile)
{
    float width = 1.0f;
    float height = 0.09f;
    Vector2 mins = Vector2(cursor.x, cursor.y - height);
    Vector2 maxs = Vector2(cursor.x + width, cursor.y);

    AABB2 bounds(mins, maxs);
    g_theRenderer->SetTexture(nullptr);
    g_theRenderer->DrawQuad2d(bounds, AABB2::ZERO_TO_ONE, Rgba(0, 0, 0, 100));

    float bar_width = width / (float)PROFILER_FRAME_HISTORY;
    Vector2 bar_cursor = bounds.mins;

    thread_visualizer_settings_t* settings = find_or_create_thread_settings(profile->m_id);

    MeshBuilder mb;
    for(int i = 0; i < PROFILER_FRAME_HISTORY; i++){
        std::shared_ptr<profiler_node_t> frame = profile->m_saved_trees[i];
        if(nullptr == frame.get()){
            bar_cursor.x += bar_width;
            continue;
        }

        float elapsed = (float)perf_counter_to_seconds(frame->end_counter - frame->start_counter);

        float fps = 1.0f / elapsed;
        float scale = Clamp((fps / 60.0f), 0.0f, 1.0f);

        AABB2 bar_bounds;
        bar_bounds.mins = bar_cursor;
        bar_bounds.maxs = bar_bounds.mins + Vector2(bar_width, height * scale);

        if(fps >= 58.0f){
            Meshes::build_quad_2d(mb, bar_bounds, AABB2::ZERO_TO_ONE, Rgba(0, 255, 0, 200), Rgba(0, 255, 0, 200));
        }else if(fps >= 30){
            Meshes::build_quad_2d(mb, bar_bounds, AABB2::ZERO_TO_ONE, Rgba::YELLOW, Rgba::YELLOW);
        }else if(fps >= 15){
            Meshes::build_quad_2d(mb, bar_bounds, AABB2::ZERO_TO_ONE, Rgba::ORANGE, Rgba::ORANGE);
        }else{
            Meshes::build_quad_2d(mb, bar_bounds, AABB2::ZERO_TO_ONE, Rgba::RED, Rgba::RED);
        }

        if(bar_bounds_clicked(bar_bounds)){
            profiler_pause_thread(profile->m_id);
            settings->force_frame = true;
            settings->frame_number_selected = i;
        }

        if(settings->force_frame && settings->frame_number_selected == i){
            bar_bounds.maxs.y = bar_bounds.mins.y + height + 0.010f;
            bar_bounds.mins.y -= 0.010f;
            Meshes::build_quad_2d(mb, bar_bounds, AABB2::ZERO_TO_ONE, Rgba::PINK, Rgba::PINK);

            g_theRenderer->DrawText2d(bar_bounds.mins, FONT_SCALE, Rgba::PINK, Stringf("%i", PROFILER_FRAME_HISTORY - 1 - settings->frame_number_selected), get_engine_font());
            g_theRenderer->SetTexture(nullptr);
        }

        bar_cursor.x += bar_width;
    }

    g_theRenderer->draw_with_meshbuilder(mb);

    if(settings->force_frame){
        if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F2)){
            settings->frame_number_selected--;
        }
        else if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F3)){
            settings->frame_number_selected++;
        }

        settings->frame_number_selected = Clamp(settings->frame_number_selected, 0, PROFILER_FRAME_HISTORY - 1);
    }

    // draw line across the graph
    {
        float avg_fps = profile->calc_avg_fps();
        float height_scale = Clamp(avg_fps / 60.0f, 0.0f, 1.0f);

        float y = bounds.mins.y + height * height_scale;

        AABB2 avg_fps_bounds(Vector2(bounds.mins.x, y), Vector2(bounds.maxs.x + 0.05f, y + 0.002f));
        g_theRenderer->DrawQuad2d(avg_fps_bounds, AABB2::ZERO_TO_ONE, Rgba::PINK.GetScaledAlpha(0.5f));

        Vector2 right_center = avg_fps_bounds.maxs + Vector2(0.01f, get_engine_font().GetLineHeight(FONT_SCALE * 0.9f) * 0.5f);
        g_theRenderer->DrawText2d(right_center, FONT_SCALE * 0.9f, Rgba::PINK, Stringf("Avg: %.2f", avg_fps), get_engine_font());
    }

    {
        float cur_fps;
        if(settings->force_frame){
            profiler_node_t* frame = profile->m_saved_trees[settings->frame_number_selected].get();
            cur_fps = 1.0f / (float)(perf_counter_to_seconds(frame->end_counter - frame->start_counter)); 
        }else{
            cur_fps = profile->calc_last_frame_fps();
        }

        float height_scale = Clamp(cur_fps / 60.0f, 0.0f, 1.0f);

        float y = bounds.mins.y + height * height_scale;

        AABB2 avg_fps_bounds(Vector2(bounds.mins.x, y), Vector2(bounds.maxs.x + 0.15f, y + 0.002f));
        g_theRenderer->SetTexture(nullptr);
        g_theRenderer->DrawQuad2d(avg_fps_bounds, AABB2::ZERO_TO_ONE, Rgba::PINK.GetScaledAlpha(0.5f));

        Vector2 right_center = avg_fps_bounds.maxs + Vector2(0.01f, get_engine_font().GetLineHeight(FONT_SCALE * 0.9f) * 0.5f);
        g_theRenderer->DrawText2d(right_center, FONT_SCALE * 0.9f, Rgba::PINK, Stringf("Cur: %.2f", cur_fps), get_engine_font());
    }

    Vector2 fps_cursor = bounds.maxs;
    fps_cursor.x += 0.005f;

    g_theRenderer->DrawText2d(fps_cursor, 0.018f, Rgba::WHITE, "60 FPS", get_engine_font());

    fps_cursor.y = bounds.mins.y + get_engine_font().GetLineHeight(0.020f);
    g_theRenderer->DrawText2d(fps_cursor, 0.018f, Rgba::WHITE, "0 FPS", get_engine_font());

    fps_cursor = bounds.mins;
    fps_cursor.y -= 0.001f;
    g_theRenderer->DrawText2d(fps_cursor, 0.018f, Rgba::WHITE, Stringf("%i", PROFILER_FRAME_HISTORY), get_engine_font());

    fps_cursor = bounds.mins + Vector2(width - get_engine_font().GetTextWidth("0", 0.020f), 0.0f);
    fps_cursor.y -= 0.001f;
    g_theRenderer->DrawText2d(fps_cursor, 0.018f, Rgba::WHITE, "0", get_engine_font());
}
#endif

#if defined(PROFILED_BUILD)
static void draw_thread(Vector2& cursor, ThreadProfile* profile)
{
    float avg_fps = profile->calc_avg_fps();
    float cur_fps = profile->calc_last_frame_fps();

    char last_frame_time_string[20];
    pretty_print_time(last_frame_time_string, 20, profile->calc_last_frame_time_seconds());

    std::string thread_name = Stringf("    %s[id:%u]", profile->m_name, profile->m_id);
    std::string fps_string = Stringf("      Avg FPS: %.3f  Current FPS: %.3f", avg_fps, cur_fps);
    std::string frame_time_string = Stringf("      Last Frame Time: %s", last_frame_time_string);
    std::string samples_string = Stringf("      Last Frame Samples: %i", profile->calc_last_frame_sample_count());

    thread_visualizer_settings_t* settings = find_or_create_thread_settings(profile->m_id);

    cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
    g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::PINK, thread_name, get_engine_font());

    float old_x = cursor.x;
    float old_y = cursor.y;

    cursor.x += get_engine_font().GetTextWidth(thread_name, FONT_SCALE);
    
    if(!settings->is_thread_minimized){
        if(bounds_clicked(cursor, "[-]")){
            settings->is_thread_minimized = true;         
        }
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "[-]", get_engine_font());
    }else{
        if(bounds_clicked(cursor, "[+]")){
            settings->is_thread_minimized = false;         
        }
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "[+]", get_engine_font());
    }

    if(settings->is_thread_minimized){
        cursor.x = old_x;
        return;
    }

    cursor.x = old_x + get_engine_font().GetTextWidth("      ", FONT_SCALE);
    cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);

    if(ThreadProfileState::RUNNING == profile->m_current_state){
        if(bounds_clicked(cursor, "Pause")){
            profiler_pause_thread(profile->m_id);
        }
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::RED, "Pause", get_engine_font());
    } else if(ThreadProfileState::PAUSING == profile->m_current_state){
        if(bounds_clicked(cursor, "Resume")){
            profiler_resume_thread(profile->m_id);
            settings->force_frame = false;
            settings->frame_number_selected = -1;
        }
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::GREEN, "Resume", get_engine_font());

        cursor.x += get_engine_font().GetTextWidth("Resume ", FONT_SCALE);
        if(bounds_clicked(cursor, "Step")){
            profiler_step_thread(profile->m_id);
        }
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::YELLOW, "Step", get_engine_font());
    }

    cursor.x = old_x;

    cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
    g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, fps_string, get_engine_font());

    cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
    g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, frame_time_string, get_engine_font());

    cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
    g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, samples_string, get_engine_font());

    float old_align = cursor.x;
    cursor.x += get_engine_font().GetTextWidth("      ", FONT_SCALE);

    draw_tree_tool(cursor, profile);

    if(!is_tree_minimized(profile->m_id)){
        ProfilerReport report(*profile);

        std::vector<std::string> storage;
        if(is_in_tree_view_mode(profile->m_id)){
            if(settings->force_frame){
                report.create_tree_view_for_frame(settings->frame_number_selected);
            }else{
                report.create_tree_view();
            }
        }else{
            if(settings->force_frame){
                report.create_flat_view_for_frame(settings->frame_number_selected);
            }else{
                report.create_flat_view();
            }
            report.sort_by_self_time();
        }
        report.store(storage);

        for(std::string& s : storage){
            cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
            g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, s, get_engine_font());
        }
    }

    Vector2 graph_cursor(cursor.x, old_y);
    graph_cursor.x += 0.42f;

    draw_thread_graph(graph_cursor, profile);
    cursor.x = old_align;
}
#endif

void profiler_render()
{
    if(!s_profiler_shown){
        return;
    }

    g_theRenderer->EnableDepth(false, false);
    g_theRenderer->SetModel(Matrix4::IDENTITY);
    g_theRenderer->SetView(Matrix4::IDENTITY);
    g_theRenderer->SetAspectNormalizedOrtho();

    Vector2 cursor(0.05f, 0.95f);
    g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "Profiler", get_engine_font());

    cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
    g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "  Memory", get_engine_font());

    cursor.x += get_engine_font().GetTextWidth("  Memory", FONT_SCALE);

    #if !defined(TRACK_MEMORY)
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::RED, "[DISABLED]", get_engine_font());
    #else

        #if (TRACK_MEMORY == TRACK_MEMORY_BASIC)
            g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::YELLOW, "[BASIC]", get_engine_font());
        #elif (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
            g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::GREEN, "[VERBOSE]", get_engine_font());
        #endif

        draw_memory_graph(cursor);

        char bytes_string[64];

        cursor.x = 0.05f;
        cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, Stringf("    Live Allocations Count        %i", mem_get_live_alloc_count()), get_engine_font());

        cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, Stringf("    Live Allocations Bytes        %s", bytes_to_string(bytes_string, 64, mem_get_live_alloc_byte_size())), get_engine_font());

        cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, Stringf("    Highwater Bytes               %s", bytes_to_string(bytes_string, 64, mem_get_highwater_alloc_byte_size())), get_engine_font());

        cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, Stringf("    Last Frame Allocations        %i", mem_get_last_frame_alloc_count()), get_engine_font());

        cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, Stringf("    Last Frame Frees              %i", mem_get_last_frame_free_count()), get_engine_font());

        cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, Stringf("    Last Frame Allocations Bytes  %s", bytes_to_string(bytes_string, 64, mem_get_last_frame_alloc_byte_size())), get_engine_font());

    #endif

    #if !defined(PROFILED_BUILD)
        cursor.x = 0.05f;
        cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE);
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "  Threads", get_engine_font());
        cursor.x += get_engine_font().GetTextWidth("  Threads", FONT_SCALE);
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::RED, "[DISABLED]", get_engine_font());
    #else
        cursor.x = 0.05f;
        cursor.y -= get_engine_font().GetLineHeight(FONT_SCALE) * 2;
        g_theRenderer->DrawText2d(cursor, FONT_SCALE, Rgba::WHITE, "  Threads", get_engine_font());

        std::vector<ThreadProfile*> snapshots = profiler_get_all_threads_snapshot();
        for(ThreadProfile* p : snapshots){
            draw_thread(cursor, p);
        }
    #endif
}