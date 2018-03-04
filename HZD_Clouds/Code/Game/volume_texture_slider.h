#pragma once

#include "Game/slider_base.h"
#include "Engine/RHI/RHITexture3D.hpp"
#include "Engine/Math/Vector4.hpp"

class VolumeTextureSlider : public SliderBase
{
public:
    int   m_min_value;
    int   m_max_value;
    int   m_current_value;
    bool  m_is_dragging;
    RHITexture3D* m_texture;
    Vector4 m_view_mask;

public:
    VolumeTextureSlider(const char* title, int min, int max, RHITexture3D* texture, const Vector4& view_mask);
    virtual ~VolumeTextureSlider() override;

    virtual void update(float ds) override;
    virtual void render(Vector2* out_cursor) override;

    virtual float get_width() override;
    virtual float get_height() override;

    void handle_mouse_down(float x, float y, float radius);
};

