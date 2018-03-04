#include "Game/slider_group.h"
#include "Game/int_slider.h"
#include "Game/float_slider.h"
#include "Game/volume_texture_slider.h"
#include "Engine/Engine.hpp"
#include "Engine/Renderer/Font.hpp"

SliderGroup::SliderGroup()
    :m_title(nullptr)
    ,m_top_left_anchor(0.0f, 0.0f)
{
}

SliderGroup::~SliderGroup()
{
    for(unsigned int i = 0; i < (unsigned int)m_sliders.size(); i++){
        SAFE_DELETE(m_sliders[i]);
    }    
}

void SliderGroup::set_title(const char* new_title)
{
    m_title = new_title;
}

void SliderGroup::set_anchor(const Vector2& top_left_anchor)
{
    m_top_left_anchor = top_left_anchor;
}

void SliderGroup::add_slider(const char* slider_title, float min_value, float max_value, float* bound_value)
{
    m_sliders.push_back(new FloatSlider(slider_title, min_value, max_value, bound_value));
}

void SliderGroup::add_slider(const char* slider_title, int min_value, int max_value, int* bound_value)
{
    m_sliders.push_back(new IntSlider(slider_title, min_value, max_value, bound_value));
}

void SliderGroup::add_slider(const char* slider_title, int min_value, int max_value, RHITexture3D* texture, const Vector4& view_mask)
{
    m_sliders.push_back(new VolumeTextureSlider(slider_title, min_value, max_value, texture, view_mask));
}

void SliderGroup::update(float ds)
{
    for(SliderBase* s : m_sliders){
        s->update(ds);
    }
}

#define PADDING 0.01f
#define TITLE_FONT_SCALE 0.02f

void SliderGroup::render()
{
    float height = 0.0f;
    float width = 0.0f;
    // render background overlay (need width and height on entire group)

    if(nullptr != m_title){
        height += get_engine_font().GetTextHeight(m_title, TITLE_FONT_SCALE);
        height += PADDING;

        float title_width = get_engine_font().GetTextWidth(m_title, TITLE_FONT_SCALE);
        width = max(width, title_width);
    }

    for(SliderBase* s : m_sliders){
        height += s->get_height();
        width = max(width, s->get_width());
    }

    height += 0.075f + PADDING;

    // padding around borders
    height += (PADDING * 2.0f); 
    width += (PADDING * 2.0f);

    Vector2 mins(m_top_left_anchor.x, m_top_left_anchor.y - height);
    Vector2 maxs = mins + Vector2(width, height);
    AABB2 bounds(mins, maxs);

    g_theRenderer->EnableDepth(false, false);
    g_theRenderer->SetModel(Matrix4::IDENTITY);
    g_theRenderer->SetView(Matrix4::IDENTITY);
    g_theRenderer->SetAspectNormalizedOrtho();
    g_theRenderer->SetTexture(nullptr);
    g_theRenderer->SetShader(nullptr);
    g_theRenderer->DrawQuad2d(bounds, AABB2::ZERO_TO_ONE, Rgba(0,0,0,200));

    Vector2 cursor = m_top_left_anchor + Vector2(PADDING, -PADDING);

    // render title
    g_theRenderer->DrawText2d(cursor, TITLE_FONT_SCALE, Rgba::WHITE, m_title, get_engine_font());
    cursor.y -= get_engine_font().GetTextHeight(m_title, TITLE_FONT_SCALE) + PADDING;

    // render each slider
    for(SliderBase* s : m_sliders){
        s->render(&cursor);
        cursor.y -= PADDING;
    }
}