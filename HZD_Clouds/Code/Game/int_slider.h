#pragma once

#include "Game/slider_base.h"

class IntSlider : public SliderBase
{
public:
    int   m_min_value;
    int   m_max_value;
    int*  m_bound_value;
    bool  m_is_dragging;

public:
    IntSlider(const char* title, int min, int max, int* bound);
    virtual ~IntSlider() override;

    virtual void update(float ds) override;
    virtual void render(Vector2* out_cursor) override;

    virtual float get_width() override;
    virtual float get_height() override;

    void handle_mouse_down(float x, float y, float radius);
};
