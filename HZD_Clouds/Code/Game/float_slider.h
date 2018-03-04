#pragma once

#include "Game/slider_base.h"

class FloatSlider : public SliderBase
{
public:
    float   m_min_value;
    float   m_max_value;
    float*  m_bound_value;
    bool    m_is_dragging;

public:
    FloatSlider(const char* title, float min, float max, float* bound);
    virtual ~FloatSlider() override;

    virtual void update(float ds) override;
    virtual void render(Vector2* out_cursor) override;

    virtual float get_width() override;
    virtual float get_height() override;

    void handle_mouse_down(float x, float y, float radius);
};