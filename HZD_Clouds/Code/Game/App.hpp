#pragma once

#include "Game/slider_group.h"

#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/Math/Vector4.hpp"
#include <vector>

class Font;
class ShaderProgram;
class ConstantBuffer;
class RHITexture3D;

struct cloud_parameters_data_t
{
    Matrix4     transform;

    float       focal_length;
    float       scale;
    float       coverage;
    float       cloud_layer_top_y;

    float       cloud_layer_bottom_y;
    Vector3     sun_position;

    float       light_step_size;
    float       detail_scale;
    Vector2     padding;

    Vector4     stratus_grad;
    Vector4     cumulus_grad;
    Vector4     cumulonumbis_grad;
    Vector4     wind;
};

struct noise_gen_paramters_data_t
{
    unsigned int    start_slice;
    float           worley_scale;
    unsigned int    worley_num_octaves;
    float           worley_octave_persistence;

    float           worley_octave_scale;
    float           perlin_scale;
    unsigned int    perlin_num_octaves;
    float           perlin_octave_persistence;

    float           perlin_octave_scale;
    float           worley_one_scale;
    float           worley_two_scale;
    float           worley_three_scale;

    unsigned int    worley_only_num_octaves;
    float           worley_only_octave_persistence;
    float           worley_only_octave_scale;
    float           detail_worley_one_scale;

    float           detail_worley_two_scale;
    float           detail_worley_three_scale;
    unsigned int    detail_worley_num_octaves;
    float           detail_worley_octave_persistence;

    float           detail_worley_octave_scale;
    float           generation_all_scale;
    float           generation_z_scale;
    float           padding;

    Vector4         view_mask;
};

class App
{
public:
	bool m_isQuitting;
	bool m_hasFocus;
	float m_deltaSeconds;
	double m_timeOfLastFrameSeconds;

	float m_viewFov;

public:
	Font*				m_appFont;
	Rgba				m_clear_color;

    ShaderProgram*      m_base_noise_gen_shader;
    ShaderProgram*      m_detail_noise_gen_shader;
    ShaderProgram*      m_clouds_shader;
    ShaderProgram*      m_old_clouds_shader;
    ShaderProgram*      m_new_clouds_shader;
    ShaderProgram*      m_slice_shader;

    cloud_parameters_data_t     m_cloud_data;
    ConstantBuffer*             m_cloud_data_buffer;

    noise_gen_paramters_data_t  m_noise_gen_data;
    ConstantBuffer*             m_noise_gen_parameters_buffer;

    RHITexture3D*       m_cloud_base;
    RHITexture3D*       m_cloud_detail;

    SliderGroup*        m_base_perlin_worley_slider;
    SliderGroup*        m_base_fbm_worley_slider;
    SliderGroup*        m_detail_noise_slider;
    SliderGroup*        m_cloud_settings_slider;

    SliderGroup*        m_perlin_worley_r;
    SliderGroup*        m_perlin_worley_g;
    SliderGroup*        m_perlin_worley_b;
    SliderGroup*        m_perlin_worley_a;

    bool                m_regen_base_noise;
    bool                m_regen_detail_noise;

    bool                m_render_sliders_overlap;
    bool                m_force_show_mouse;

	float				m_sun_pos;
	float				m_sun_distance;

public:
	App();
	~App();

    void init();
    void init_vis_settings();
    void init_sliders();
	void InitRendering();

	void Run();

public:
	void SetIsQuitting(bool isQuitting) { m_isQuitting = isQuitting; }
	bool IsRunning() const { return !m_isQuitting; }
	bool IsQuitting() const { return m_isQuitting; }

	void RegisterKeyDown(unsigned char keyCode);
	void RegisterKeyUp(unsigned char keyCode);

	void RegisterMouseWheelDelta(float mouseWheelDelta);

	void OnGainedFocus();
	void OnLostFocus();
	bool HasFocus() const { return m_hasFocus; }

public:
	void RunFrame();

	void BeginFrame();
	void Update(float deltaSeconds);
	void Render();
	void EndFrame();

	void StepTime();
	void BurnLeftOverFrameTime();

public:
    void render_regen_base_noise();
    void render_regen_detail_noise();
    void render_clouds();
};