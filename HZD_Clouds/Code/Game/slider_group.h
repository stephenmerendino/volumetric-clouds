#pragma once

#include "Game/slider_base.h"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/RHI/RHITexture3D.hpp"
#include <vector>

class SliderGroup
{
public:
    const char*                 m_title;
    Vector2                     m_top_left_anchor;
    std::vector<SliderBase*>    m_sliders;

public:
    SliderGroup();
    ~SliderGroup();

    void set_title(const char* new_title);
    void set_anchor(const Vector2& top_left_anchor);
    void set_anchor(float x, float y){ set_anchor(Vector2(x, y)); };
    void add_slider(const char* slider_title, float min_value, float max_value, float* bound_value);
    void add_slider(const char* slider_title, int min_value, int max_value, int* bound_value);
    void add_slider(const char* slider_title, int min_value, int max_value, RHITexture3D* texture, const Vector4& view_mask);

    void update(float ds);
    void render();
};