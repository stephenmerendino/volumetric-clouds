#include "Game/volume_texture_slider.h"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"

VolumeTextureSlider::VolumeTextureSlider(const char* title, int min, int max, RHITexture3D* texture, const Vector4& view_mask)
    :SliderBase(title)
    ,m_min_value(min)
    ,m_max_value(max)
    ,m_current_value(0)
    ,m_texture(texture)
    ,m_is_dragging(false)
    ,m_view_mask(view_mask)
{
}

VolumeTextureSlider::~VolumeTextureSlider()
{
}

void VolumeTextureSlider::update(float ds)
{
    UNUSED(ds);
    if(m_is_dragging && !g_theInputSystem->IsKeyDown(KEYCODE_LMB)){
        m_is_dragging = false;
    }
}

void VolumeTextureSlider::render(Vector2* out_cursor)
{
    Vector2 cursor_copy = *out_cursor;
    AABB2 bounds = calc_bounds(cursor_copy);

    // Draw the title
    g_theRenderer->DrawText2d(cursor_copy, SLIDER_FONT_SCALE, Rgba::WHITE, m_title, get_engine_font());
    cursor_copy.y -= get_engine_font().GetTextHeight(m_title, SLIDER_FONT_SCALE);

    // Draw the slider bar
    float bar_thickness = 0.0015f;
    AABB2 bar_bounds = bounds;
    bar_bounds.Scale(0.85f);
    Vector2 center = bar_bounds.CalcCenter();
    center.y += 0.0325f;
    bar_bounds.mins.y = center.y - bar_thickness;
    bar_bounds.maxs.y = center.y + bar_thickness;

    g_theRenderer->SetShader(nullptr);
    g_theRenderer->SetTexture(nullptr);
    g_theRenderer->DrawQuad2d(bar_bounds, AABB2::ZERO_TO_ONE, Rgba(255, 255, 255, 200));

    // Draw the min and max values
    std::string min_text = Stringf("%i", m_min_value);
    std::string max_text = Stringf("%i", m_max_value);

    float min_max_text_height = get_engine_font().GetTextHeight(min_text, SLIDER_FONT_SCALE * 0.9f); 
    Vector2 min_cursor = bar_bounds.CalcTopLeft() + Vector2(0.0f, min_max_text_height + 0.0085f);
    g_theRenderer->DrawText2d(min_cursor, SLIDER_FONT_SCALE * 0.9f, Rgba::WHITE, min_text, get_engine_font());

    float max_width = get_engine_font().GetTextWidth(max_text, SLIDER_FONT_SCALE * 0.9f);
    Vector2 max_cursor = bar_bounds.maxs + Vector2(-max_width, min_max_text_height + 0.0085f);
    g_theRenderer->DrawText2d(max_cursor, SLIDER_FONT_SCALE * 0.9f, Rgba::WHITE, max_text, get_engine_font());

    if(m_is_dragging){
        Vector2 mouse = g_theInputSystem->GetCursorClientAreaNormalized();
        mouse.x *= g_theRenderer->m_output->GetAspectRatio();
        float min_x = bar_bounds.mins.x;
        float max_x = bar_bounds.maxs.x;
        float cur_value = MapFloatToRange(mouse.x, min_x, max_x, 0.0f, 1.0f);
        cur_value = Clamp(cur_value, 0.0f, 1.0f);
        m_current_value = (int)(MapFloatToRange(cur_value, 0.0f, 1.0f, (float)m_min_value, (float)m_max_value));
    }

    // Draw the circle where it goes in the range
    float radius = 0.0075f;
    float normalized_pos = MapFloatToRange((float)m_current_value, (float)m_min_value, (float)m_max_value, 0.0f, 1.0f);
    float x_pos = bar_bounds.mins.x + (bar_bounds.CalcWidth() * normalized_pos);
    float y_pos = bar_bounds.CalcCenter().y;
    if(m_is_dragging){
        g_theRenderer->DebugDrawCircle2d(Vector2(x_pos, y_pos), radius, 0.0f, 0.0f, Rgba::PINK, Rgba::PINK, 32);
    }else{
        g_theRenderer->DebugDrawCircle2d(Vector2(x_pos, y_pos), radius, 0.0f, 0.0f, Rgba::PINK.GetScaledAlpha(0.5f), Rgba::PINK.GetScaledAlpha(0.5f), 32);
    }

    handle_mouse_down(x_pos, y_pos, radius);

    y_pos -= min_max_text_height;
    
    std::string cur_text = Stringf("%i", m_current_value);
    g_theRenderer->DrawText2dCentered(Vector2(x_pos, y_pos), SLIDER_FONT_SCALE * 0.9f, Rgba::PINK, cur_text, get_engine_font());

    // also render the texture at the current slice
    g_theRenderer->SetShaderProgram(g_theApp->m_slice_shader);
    g_theApp->m_noise_gen_data.start_slice = m_current_value;
    g_theApp->m_noise_gen_data.view_mask = m_view_mask;
    g_theRenderer->SetConstantBuffer(4, g_theApp->m_noise_gen_parameters_buffer);
    g_theRenderer->UpdateConstantBuffer(g_theApp->m_noise_gen_parameters_buffer, &g_theApp->m_noise_gen_data);
    g_theRenderer->m_deviceContext->SetTexture(0, m_texture);

    // figure out the bounds of the texture draw

    Vector2 mins;
    Vector2 maxs;

    AABB2 test_bounds = calc_bounds(*out_cursor);

    mins.x = test_bounds.mins.x;
    mins.y = test_bounds.mins.y - 0.09f;
    maxs.x = test_bounds.maxs.x;
    maxs.y = y_pos - 0.005f;

    AABB2 tex_bounds(mins, maxs);

	//tex_bounds = AABB2::ZERO_TO_ONE;
	//g_theRenderer->SetOrthoProjection(tex_bounds);
	//g_theRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

    g_theRenderer->DrawQuad2d(tex_bounds);

    // final height
    out_cursor->y -= get_height();
}

float VolumeTextureSlider::get_width()
{
    return 0.25f;
}

float VolumeTextureSlider::get_height()
{
    return 0.14f;
}

void VolumeTextureSlider::handle_mouse_down(float x, float y, float radius)
{
    if(!g_theInputSystem->WasKeyJustPressed(KEYCODE_LMB)){
        return;
    }

    Vector2 mouse = g_theInputSystem->GetCursorClientAreaNormalized();
    mouse.x *= g_theRenderer->m_output->GetAspectRatio();

    Disc2 grabber(x, y, radius);
    if(grabber.IsPointInside(mouse)){
        m_is_dragging = true;
    }
}