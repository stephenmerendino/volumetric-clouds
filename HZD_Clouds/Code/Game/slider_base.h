#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"

#define SLIDER_FONT_SCALE 0.019f

class SliderBase
{
public:
    const char* m_title;

public:
    SliderBase(const char* title = nullptr)
        :m_title(title)
    {}

    virtual ~SliderBase(){};

    virtual void set_title(const char* new_title){ m_title = new_title; }

    virtual void update(float ds) = 0;
    virtual void render(Vector2* out_cursor) = 0;

    virtual float get_width() = 0;
    virtual float get_height() = 0;

    AABB2 calc_bounds(const Vector2& top_left);
};
