#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameConfig.hpp"
#include "Game/Camera3d.hpp"

#include "Engine/Engine.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Config.hpp"
#include "Engine/Core/Display.hpp"
#include "Engine/Core/Console.hpp"
#include "Engine/Core/log.h"
#include "Engine/Input/midi.h"

#include "Engine/Math/Noise.hpp"

#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/RHITexture3D.hpp"

#include "Engine/Renderer/Font.hpp"

#include "Engine/Tools/fbx.hpp"

COMMAND(set_scale, "Set cloud sample scale")
{
    g_theApp->m_cloud_data.scale = args.next_float_arg();
}

COMMAND(set_coverage, "Set cloud coverage")
{
    g_theApp->m_cloud_data.coverage = args.next_float_arg();
}

COMMAND(set_light_step, "Sets the step size of the lighting equation")
{
    g_theApp->m_cloud_data.light_step_size = args.next_float_arg();
}

COMMAND(set_cloud_top, "Sets the top y position of the cloud layer")
{
    g_theApp->m_cloud_data.cloud_layer_top_y = args.next_float_arg();
}

COMMAND(set_cloud_bottom, "Sets the top y position of the cloud layer")
{
    g_theApp->m_cloud_data.cloud_layer_bottom_y = args.next_float_arg();
}

COMMAND(set_detail_scale, "Sets the detail scale")
{
    g_theApp->m_cloud_data.detail_scale = args.next_float_arg();
}

COMMAND(set_wind_speed, "Sets the wind speed")
{
    g_theApp->m_cloud_data.wind.w = args.next_float_arg();
}

COMMAND(save_clouds, "Saves clouds")
{    
    ConfigSet("base_worley_scale", g_theApp->m_noise_gen_data.worley_scale);
    ConfigSet("base_worley_num_octaves", (int)g_theApp->m_noise_gen_data.worley_num_octaves);
    ConfigSet("base_worley_octave_persistence", g_theApp->m_noise_gen_data.worley_octave_persistence);
    ConfigSet("base_worley_octave_scale", g_theApp->m_noise_gen_data.worley_octave_scale);

    ConfigSet("base_perlin_scale", g_theApp->m_noise_gen_data.perlin_scale);
    ConfigSet("base_perlin_num_octaves", (int)g_theApp->m_noise_gen_data.perlin_num_octaves);
    ConfigSet("base_perlin_octave_persistence", g_theApp->m_noise_gen_data.perlin_octave_persistence);
    ConfigSet("base_perlin_octave_scale", g_theApp->m_noise_gen_data.perlin_octave_scale);
    ConfigSet("generation_xyz_scale", g_theApp->m_noise_gen_data.generation_all_scale);
    ConfigSet("generation_z_scale", g_theApp->m_noise_gen_data.generation_z_scale);

    ConfigSet("base_worley_one_scale", g_theApp->m_noise_gen_data.worley_one_scale);
    ConfigSet("base_worley_two_scale", g_theApp->m_noise_gen_data.worley_two_scale);
    ConfigSet("base_worley_three_scale", g_theApp->m_noise_gen_data.worley_three_scale);
    ConfigSet("base_fbm_worley_scale", (int)g_theApp->m_noise_gen_data.worley_only_num_octaves);
    ConfigSet("base_fbm_worley_octave_persistence", g_theApp->m_noise_gen_data.worley_only_octave_persistence);
    ConfigSet("base_fbm_worley_octave_scale", g_theApp->m_noise_gen_data.worley_only_octave_scale);

    ConfigSet("detail_worley_one_scale", g_theApp->m_noise_gen_data.detail_worley_one_scale);
    ConfigSet("detail_worley_two_scale", g_theApp->m_noise_gen_data.detail_worley_two_scale);
    ConfigSet("detail_worley_three_scale", g_theApp->m_noise_gen_data.detail_worley_three_scale);
    ConfigSet("detail_worley_num_octaves", (int)g_theApp->m_noise_gen_data.detail_worley_num_octaves);
    ConfigSet("detail_worley_octave_peristence", g_theApp->m_noise_gen_data.detail_worley_octave_persistence);
    ConfigSet("detail_worley_octave_scale", g_theApp->m_noise_gen_data.detail_worley_octave_scale);

    ConfigSet("cloud_sample_scale", g_theApp->m_cloud_data.scale);
    ConfigSet("cloud_coverage", g_theApp->m_cloud_data.coverage);
    ConfigSet("cloud_layer_top_y", g_theApp->m_cloud_data.cloud_layer_top_y);
    ConfigSet("cloud_layer_bottom_y", g_theApp->m_cloud_data.cloud_layer_bottom_y);
    ConfigSet("cloud_light_step_size", g_theApp->m_cloud_data.light_step_size);
    ConfigSet("cloud_detail_scale", g_theApp->m_cloud_data.detail_scale);

    ConfigSystemSaveToFile("Data/Config.dat");

    // TODO, write out current textures to file
    g_theApp->m_cloud_base->save_to_file("Data/Images/cloud_base_4.texture");
    g_theApp->m_cloud_detail->save_to_file("Data/Images/cloud_base_4.texture");
}

App::App()
	:m_hasFocus(true)
	,m_isQuitting(false)
	,m_deltaSeconds(0.0f)
	,m_timeOfLastFrameSeconds(0.0f)
	,m_appFont(nullptr)
	,m_viewFov(0.0f)
    ,m_clouds_shader(nullptr)
    ,m_old_clouds_shader(nullptr)
    ,m_new_clouds_shader(nullptr)
    ,m_cloud_data_buffer(nullptr)
    ,m_base_noise_gen_shader(nullptr)
    ,m_detail_noise_gen_shader(nullptr)
    ,m_slice_shader(nullptr)
    ,m_regen_base_noise(false)
    ,m_regen_detail_noise(false)
    ,m_base_perlin_worley_slider(nullptr)
    ,m_base_fbm_worley_slider(nullptr)
    ,m_detail_noise_slider(nullptr)
    ,m_cloud_settings_slider(nullptr)
    ,m_render_sliders_overlap(false)
    ,m_force_show_mouse(false)
    ,m_perlin_worley_r(nullptr)
    ,m_perlin_worley_g(nullptr)
    ,m_perlin_worley_b(nullptr)
    ,m_perlin_worley_a(nullptr)
	,m_sun_pos(0.25f)
	,m_sun_distance(10000000.f)
{
	g_theGame = new Game();
}

App::~App()
{
    SAFE_DELETE(m_perlin_worley_r);
    SAFE_DELETE(m_perlin_worley_g);
    SAFE_DELETE(m_perlin_worley_b);
    SAFE_DELETE(m_perlin_worley_a);
    SAFE_DELETE(m_base_perlin_worley_slider);
    SAFE_DELETE(m_base_fbm_worley_slider);
    SAFE_DELETE(m_detail_noise_slider);
    SAFE_DELETE(m_cloud_settings_slider);
    SAFE_DELETE(m_cloud_base);
    SAFE_DELETE(m_cloud_detail);
    SAFE_DELETE(m_cloud_data_buffer);
    SAFE_DELETE(m_old_clouds_shader);
    SAFE_DELETE(m_new_clouds_shader);
    SAFE_DELETE(m_base_noise_gen_shader);
    SAFE_DELETE(m_detail_noise_gen_shader);
    SAFE_DELETE(m_slice_shader);
	SAFE_DELETE(m_appFont);
	SAFE_DELETE(g_theGame);
}

void App::Run()
{
	while(IsRunning()){
		RunFrame();
	}
}

void App::init()
{
    m_cloud_base = new RHITexture3D(g_theRenderer->m_device);
	m_cloud_base->m_generate_mips = true;
    m_cloud_base->LoadFromFilenameRGBA8("Data/Images/gg_base_noise.dat", 128, 128, 128);

    m_cloud_detail = new RHITexture3D(g_theRenderer->m_device);
	m_cloud_detail->m_generate_mips = true;
    m_cloud_detail->LoadFromFilenameRGBA8("Data/Images/gg_detail_noise.dat", 32, 32, 32);

    init_vis_settings();
    init_sliders();
    InitRendering();
}

void App::init_sliders()
{
    m_base_perlin_worley_slider = new SliderGroup();
    m_base_perlin_worley_slider->set_title("Base Perlin-Worley");
    m_base_perlin_worley_slider->set_anchor(0.025f, 0.975f);

    m_base_perlin_worley_slider->add_slider("Worley Scale", 0.0f, 1.0f, &m_noise_gen_data.worley_scale);
    m_base_perlin_worley_slider->add_slider("Worley Num Octaves", 0, 8, (int*)&m_noise_gen_data.worley_num_octaves);
    m_base_perlin_worley_slider->add_slider("Worley Octave Persistence", 0.0f, 4.0f, (float*)&m_noise_gen_data.worley_octave_persistence);
    m_base_perlin_worley_slider->add_slider("Worley Octave Scale", 0.0f, 4.0f, (float*)&m_noise_gen_data.worley_octave_scale);

    m_base_perlin_worley_slider->add_slider("Perlin Scale", 0.0f, 50.0f, (float*)&m_noise_gen_data.perlin_scale);
    m_base_perlin_worley_slider->add_slider("Perlin Num Octaves", 0, 8, (int*)&m_noise_gen_data.perlin_num_octaves);
    m_base_perlin_worley_slider->add_slider("Perlin Octave Persistence", 0.0f, 4.0f, (float*)&m_noise_gen_data.perlin_octave_persistence);
    m_base_perlin_worley_slider->add_slider("Perlin Octave Scale", 0.0f, 4.0f, (float*)&m_noise_gen_data.perlin_octave_scale);
    m_base_perlin_worley_slider->add_slider("Generation XYZ-Scale", 0.0f, 20.0f, (float*)&m_noise_gen_data.generation_all_scale);
    m_base_perlin_worley_slider->add_slider("Generation Z-Scale", 0.0f, 150.0f, (float*)&m_noise_gen_data.generation_z_scale);

    m_base_fbm_worley_slider = new SliderGroup();
    m_base_fbm_worley_slider->set_title("Base FBM Worley");
    m_base_fbm_worley_slider->set_anchor(0.4f, 0.975f);
    m_base_fbm_worley_slider->add_slider("(G) Worley One Scale", 0.0f, 0.5f, (float*)&m_noise_gen_data.worley_one_scale);
    m_base_fbm_worley_slider->add_slider("(B) Worley Two Scale", 0.0f, 0.5f, (float*)&m_noise_gen_data.worley_two_scale);
    m_base_fbm_worley_slider->add_slider("(A) Worley Three Scale", 0.0f, 0.5f, (float*)&m_noise_gen_data.worley_three_scale);
    m_base_fbm_worley_slider->add_slider("Num Octaves", 0, 8, (int*)&m_noise_gen_data.worley_only_num_octaves);
    m_base_fbm_worley_slider->add_slider("Octave Persistence", 0.0f, 4.0f, (float*)&m_noise_gen_data.worley_only_octave_persistence);
    m_base_fbm_worley_slider->add_slider("Octave Scale", 0.0f, 4.0f, (float*)&m_noise_gen_data.worley_only_octave_scale);

    m_detail_noise_slider = new SliderGroup();
    m_detail_noise_slider->set_title("Detail Worley Noise");
    m_detail_noise_slider->set_anchor(0.775f, 0.975f);
    m_detail_noise_slider->add_slider("(R) Worley One Scale", 0.0f, 10.0f, (float*)&m_noise_gen_data.detail_worley_one_scale);
    m_detail_noise_slider->add_slider("(G) Worley Two Scale", 0.0f, 20.0f, (float*)&m_noise_gen_data.detail_worley_two_scale);
    m_detail_noise_slider->add_slider("(B) Worley Three Scale", 0.0f, 30.0f, (float*)&m_noise_gen_data.detail_worley_three_scale);
    m_detail_noise_slider->add_slider("Num Octaves", 0, 8, (int*)&m_noise_gen_data.detail_worley_num_octaves);
    m_detail_noise_slider->add_slider("Octave Persistence", 0.0f, 4.0f, (float*)&m_noise_gen_data.detail_worley_octave_persistence);
    m_detail_noise_slider->add_slider("Octave Scale", 0.0f, 4.0f, (float*)&m_noise_gen_data.detail_worley_octave_scale);

    m_cloud_settings_slider = new SliderGroup();
    m_cloud_settings_slider->set_anchor(1.150f, 0.975f);
    m_cloud_settings_slider->set_title("Cloud Settings");
    m_cloud_settings_slider->add_slider("Sample Scale", 0.0f, 0.00010f, (float*)&m_cloud_data.scale);
    m_cloud_settings_slider->add_slider("Coverage", 0.0f, 1.0f, (float*)&m_cloud_data.coverage);
    m_cloud_settings_slider->add_slider("Cloud Layer Top Y", 0.0f, 50000.0f, (float*)&m_cloud_data.cloud_layer_top_y);
    m_cloud_settings_slider->add_slider("Cloud Layer Bottom Y", 0.0f, 50000.0f, (float*)&m_cloud_data.cloud_layer_bottom_y);
    m_cloud_settings_slider->add_slider("Light Step Size", 0.0f, 5000.0f, (float*)&m_cloud_data.light_step_size);
    m_cloud_settings_slider->add_slider("Detail Scale", 0.0f, 0.01f, (float*)&m_cloud_data.detail_scale);
    m_cloud_settings_slider->add_slider("Wind Speed", 0.0f, 2000.00f, (float*)&m_cloud_data.wind.w);

    m_perlin_worley_r = new SliderGroup();
    m_perlin_worley_r->set_title("Perlin-Worley R");
    m_perlin_worley_r->set_anchor(0.4f, 0.345f);
    m_perlin_worley_r->add_slider("Slice", 0, 127, m_cloud_base, Vector4(1.0f, 0.0f, 0.0f, 0.0f));

    m_perlin_worley_g = new SliderGroup();
    m_perlin_worley_g->set_title("Perlin-Worley G");
    m_perlin_worley_g->set_anchor(0.675f, 0.345f);
    m_perlin_worley_g->add_slider("Slice", 0, 127, m_cloud_base, Vector4(0.0f, 1.0f, 0.0f, 0.0f));

    m_perlin_worley_b = new SliderGroup();
    m_perlin_worley_b->set_title("Perlin-Worley B");
    m_perlin_worley_b->set_anchor(0.95f, 0.345f);
    m_perlin_worley_b->add_slider("Slice", 0, 127, m_cloud_base, Vector4(0.0f, 0.0f, 1.0f, 0.0f));

    m_perlin_worley_a = new SliderGroup();
    m_perlin_worley_a->set_title("Perlin-Worley A");
    m_perlin_worley_a->set_anchor(1.225f, 0.345f);
    m_perlin_worley_a->add_slider("Slice", 0, 127, m_cloud_base, Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	midi_bindf_knob(4, &m_sun_pos, 0.0f, 1.0f);
	//midi_bindf_knob(5, &m_sun_distance, 0.0f, 100000000.0f);

    //midi_bindf_slider(0, &m_cloud_data.scale, 0.0f, 0.00025f);
    //midi_bindf_slider(1, &m_cloud_data.coverage, 0.0f, 1.000f);
    //midi_bindf_slider(2, &m_cloud_data.cloud_layer_bottom_y, 0.0f, 50000.0f);
    //midi_bindf_slider(3, &m_cloud_data.cloud_layer_top_y, 0.0f, 50000.0f);
    //midi_bindf_slider(4, &m_cloud_data.light_step_size, 0.0f, 5000.0f);
    //midi_bindf_slider(5, &m_cloud_data.detail_scale, 0.0f, 0.05f);
    //midi_bindf_slider(6, &m_cloud_data.wind.w, 0.0f, 2000.0f);

    //midi_bindf_slider(7, &m_noise_gen_data.worley_scale, 0.0f, 2.0f);
    //midi_bindf_slider(8, &m_noise_gen_data.perlin_scale, 0.0f, 50.0f);

    //midi_bindi_knob(0, (int*)&m_noise_gen_data.worley_num_octaves, 0, 10);
    //midi_bindf_knob(1, &m_noise_gen_data.worley_octave_persistence, 0.0f, 4.0f);
    //midi_bindf_knob(2, &m_noise_gen_data.worley_octave_scale, 0.0f, 4.0f);
    //midi_bindi_knob(3, (int*)&m_noise_gen_data.perlin_num_octaves, 0, 8);
    //midi_bindf_knob(4, &m_noise_gen_data.perlin_octave_persistence, 0.0f, 6.0f);
    //midi_bindf_knob(5, &m_noise_gen_data.perlin_octave_scale, 0.0f, 6.0f);
    //midi_bindf_knob(6, &m_noise_gen_data.generation_all_scale, 0.0f, 25.0f);
    //midi_bindf_knob(7, &m_noise_gen_data.generation_z_scale, 0.0f, 150.0f);

    //midi_bind_button(0, &m_render_sliders_overlap);
}

// TODO, load from config
void App::init_vis_settings()
{
    // DEFAULTS
    // Setup initial noise generation data
    m_noise_gen_data.start_slice                        = 0;
    m_noise_gen_data.worley_scale                       = 0.10f;
    m_noise_gen_data.worley_num_octaves                 = 4;
    m_noise_gen_data.worley_octave_persistence          = 0.5f;
    m_noise_gen_data.worley_octave_scale                = 2.0f;
    m_noise_gen_data.perlin_scale                       = 12.5f;
    m_noise_gen_data.perlin_num_octaves                 = 4;
    m_noise_gen_data.perlin_octave_persistence          = 2.0f;
    m_noise_gen_data.perlin_octave_scale                = 0.5f;

    m_noise_gen_data.worley_one_scale                   = 0.15f;
    m_noise_gen_data.worley_two_scale                   = 0.25f;
    m_noise_gen_data.worley_three_scale                 = 0.35f;
    m_noise_gen_data.worley_only_num_octaves            = 4;
    m_noise_gen_data.worley_only_octave_persistence     = 0.5f;
    m_noise_gen_data.worley_only_octave_scale           = 2.0f;

    m_noise_gen_data.detail_worley_one_scale            = 1.5f;
    m_noise_gen_data.detail_worley_two_scale            = 2.5f;
    m_noise_gen_data.detail_worley_three_scale          = 3.5f;
    m_noise_gen_data.detail_worley_num_octaves          = 4;
    m_noise_gen_data.detail_worley_octave_persistence   = 0.5f;
    m_noise_gen_data.detail_worley_octave_scale         = 2.0f;

    m_noise_gen_data.generation_all_scale               = 1.0f;
    m_noise_gen_data.generation_z_scale                 = 50.0f;

    // Setup initial cloud data
    m_cloud_data.scale                                  = 0.000025f;
    m_cloud_data.coverage                               = 0.585002170f;
    m_cloud_data.cloud_layer_top_y                      = 6000.f;
    m_cloud_data.cloud_layer_bottom_y                   = 4500.f;
    m_cloud_data.sun_position                           = Vector3(10000000.0f, 1000000.0f, 10000000.0f);
    m_cloud_data.light_step_size                        = 100.0f;
    m_cloud_data.detail_scale                           = 0.00005f;
    m_cloud_data.stratus_grad                           = Vector4(0.05f, 0.08f, 0.14f, 0.16f);
    m_cloud_data.cumulus_grad                           = Vector4(0.05f, 0.20f, 0.50f, 0.65f);
    m_cloud_data.cumulonumbis_grad                      = Vector4(0.05f, 0.15f, 0.8f, 0.9f);
    m_cloud_data.wind                                   = Vector4(0.0f, 0.0f, 1.0f, 0.f);

    // LOAD FROM CONFIG
    ConfigGetFloat(&m_noise_gen_data.worley_scale, "base_worley_scale");
    ConfigGetInt((int*)&m_noise_gen_data.worley_num_octaves, "base_worley_num_octaves");
    ConfigGetFloat(&m_noise_gen_data.worley_octave_persistence, "base_worley_octave_persistence");
    ConfigGetFloat(&m_noise_gen_data.worley_octave_scale, "base_worley_octave_scale");

    ConfigGetFloat(&m_noise_gen_data.perlin_scale, "base_perlin_scale");
    ConfigGetInt((int*)&m_noise_gen_data.perlin_num_octaves, "base_perlin_num_octaves");
    ConfigGetFloat(&m_noise_gen_data.perlin_octave_persistence, "base_perlin_octave_persistence");
    ConfigGetFloat(&m_noise_gen_data.perlin_octave_scale, "base_perlin_octave_scale");
    ConfigGetFloat(&m_noise_gen_data.generation_all_scale,"generation_xyz_scale");
    ConfigGetFloat(&m_noise_gen_data.generation_z_scale,"generation_z_scale");

    ConfigGetFloat(&m_noise_gen_data.worley_one_scale, "base_worley_one_scale");
    ConfigGetFloat(&m_noise_gen_data.worley_two_scale, "base_worley_two_scale");
    ConfigGetFloat(&m_noise_gen_data.worley_three_scale,"base_worley_three_scale");
    ConfigGetInt((int*)&m_noise_gen_data.worley_only_num_octaves,"base_fbm_worley_scale");
    ConfigGetFloat(&m_noise_gen_data.worley_only_octave_persistence, "base_fbm_worley_octave_persistence");
    ConfigGetFloat(&m_noise_gen_data.worley_only_octave_scale, "base_fbm_worley_octave_scale");

    ConfigGetFloat(&m_noise_gen_data.detail_worley_one_scale, "detail_worley_one_scale");
    ConfigGetFloat(&m_noise_gen_data.detail_worley_two_scale, "detail_worley_two_scale");
    ConfigGetFloat(&m_noise_gen_data.detail_worley_three_scale, "detail_worley_three_scale");
    ConfigGetInt((int*)&m_noise_gen_data.detail_worley_num_octaves, "detail_worley_num_octaves");
    ConfigGetFloat(&m_noise_gen_data.detail_worley_octave_persistence, "detail_worley_octave_peristence");
    ConfigGetFloat(&m_noise_gen_data.detail_worley_octave_scale, "detail_worley_octave_scale");

    ConfigGetFloat(&m_cloud_data.scale, "cloud_sample_scale");
    ConfigGetFloat(&m_cloud_data.coverage, "cloud_coverage");
    ConfigGetFloat(&m_cloud_data.cloud_layer_top_y, "cloud_layer_top_y");
    ConfigGetFloat(&m_cloud_data.cloud_layer_bottom_y, "cloud_layer_bottom_y");
    ConfigGetFloat(&m_cloud_data.light_step_size, "cloud_light_step_size");
    ConfigGetFloat(&m_cloud_data.detail_scale, "cloud_detail_scale");
}

void App::InitRendering()
{
	// load up config options
	ConfigGetRgba(&m_clear_color, CONFIG_CLEAR_COLOR_NAME);
	ConfigGetFloat(&m_viewFov, CONFIG_FOV_NAME);

	m_appFont = g_theRenderer->m_device->CreateFontFromFile(APP_FONT);

    m_noise_gen_parameters_buffer                       = g_theRenderer->m_device->CreateConstantBuffer(&m_noise_gen_data, sizeof(m_noise_gen_data));
    m_cloud_data_buffer                                 = g_theRenderer->m_device->CreateConstantBuffer(&m_cloud_data, sizeof(m_cloud_data));

    m_base_noise_gen_shader     = g_theRenderer->m_device->CreateShaderProgramFromFiles("Data/HLSL/mvp.vert", "Data/HLSL/3d_noise_slices.geom", "Data/HLSL/3d_base_noise.frag");
    m_detail_noise_gen_shader   = g_theRenderer->m_device->CreateShaderProgramFromFiles("Data/HLSL/mvp.vert", "Data/HLSL/3d_noise_slices.geom", "Data/HLSL/3d_detail_noise.frag");
    m_old_clouds_shader         = g_theRenderer->m_device->CreateShaderProgramFromFiles("Data/HLSL/mvp.vert", nullptr, "Data/HLSL/hzd_clouds.frag");
    m_new_clouds_shader         = g_theRenderer->m_device->CreateShaderProgramFromFiles("Data/HLSL/mvp.vert", nullptr, "Data/HLSL/hzd_clouds_new.frag");
    m_slice_shader              = g_theRenderer->m_device->CreateShaderProgramFromFiles("Data/HLSL/mvp.vert", nullptr, "Data/HLSL/slice_texture.frag");

	//m_clouds_shader = m_old_clouds_shader;
	m_clouds_shader = m_new_clouds_shader;
}

void App::RegisterKeyDown(unsigned char keyCode)
{
	console_register_non_char_key_down(keyCode);

	if(!is_console_open()){
		g_theInputSystem->RegisterKeyDown(keyCode);
	}
}

void App::RegisterKeyUp(unsigned char keyCode)
{
	console_register_non_char_key_up(keyCode);

	if (!is_console_open()){
		g_theInputSystem->RegisterKeyUp(keyCode);
	}
}

void App::RegisterMouseWheelDelta(float mouseWheelDelta)
{
	if (!is_console_open()){
		g_theInputSystem->RegisterMouseWheelDelta(mouseWheelDelta);
	}
}

void App::OnGainedFocus()
{
	m_hasFocus = true;
	g_theInputSystem->ShowMouseCursor(false);
}

void App::OnLostFocus()
{
	m_hasFocus = false;
	g_theInputSystem->ShowMouseCursor(true);
}

void App::RunFrame()
{
	BeginFrame();

	if(!g_theRenderer->m_output->IsOpen()){
		SetIsQuitting(true);
		return;
	}

	Update(m_deltaSeconds);
	Render();

	EndFrame();
}

void App::BeginFrame()
{
	StepTime();

	g_theInputSystem->BeginFrame();

	// Only update mouse move delta if app has focus and dev console is not open
    if(!m_force_show_mouse){
    	if(m_hasFocus && !is_console_open()){
    		g_theInputSystem->UpdateMouseMoveDelta();
    	}

    	// If Dev console is open, show the mouse, if not open, then don't show the mouse
    	if(is_console_open()){
    		g_theInputSystem->ShowMouseCursor(true);
    	}
    	else{
    		g_theInputSystem->ShowMouseCursor(false);
    	}
    }else{
		g_theInputSystem->ShowMouseCursor(true);
    }

	console_update(m_deltaSeconds);

	g_theRenderer->m_output->ProcessMessages();
	g_theRenderer->Update(m_deltaSeconds);
}

void App::Update(float deltaSeconds)
{
	if(g_theInputSystem->WasKeyJustPressed(KEYCODE_ESCAPE)){
		SetIsQuitting(true);
	}

	g_theGame->Update(deltaSeconds);

    float focal_length = 1.0f / TanDegrees(m_viewFov / 2.0f); 

    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_UP)){
        m_cloud_data.scale += 0.0000001f;
        log_printf("Scale: %.10f", m_cloud_data.scale);
    }

    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_DOWN)){
        m_cloud_data.scale -= 0.0000001f;
        log_printf("Scale: %.10f", m_cloud_data.scale);
    }

    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_RIGHT)){
        m_cloud_data.coverage += 0.001f;
        log_printf("Coverage: %.10f", m_cloud_data.coverage);
    }

    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_LEFT)){
        m_cloud_data.coverage -= 0.001f;
        log_printf("Coverage: %.10f", m_cloud_data.coverage);
    }

    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F5)){
        m_render_sliders_overlap = !m_render_sliders_overlap;
    }

    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F6)){
        m_regen_base_noise = true;
    }

    if(g_theInputSystem->WasKeyJustPressed(KEYCODE_F7)){
        m_regen_detail_noise = true;
    }

    if(g_theInputSystem->WasKeyJustPressed('M')){
        m_force_show_mouse = !m_force_show_mouse;
        if(!m_force_show_mouse){
            g_theInputSystem->SetCursorScreenPos(g_theInputSystem->GetScreenCenter());
            g_theInputSystem->ShowMouseCursor(false);
        }
    }

	Vector4 s = Vector4(0.0f, 0.0f, 1.0f, 0.0f);

	float rot_degs = MapFloatToRange(m_sun_pos, 0.0f, 1.0f, 0.0f, -180.0f);
	Matrix4 rot = Matrix4::make_rotation_x_degrees(rot_degs);

	Vector4 new_sun_dir = s * rot;
	new_sun_dir *= 1000000.0f;

	m_cloud_data.sun_position = new_sun_dir.xyz;

    m_base_perlin_worley_slider->update(deltaSeconds);
    m_base_fbm_worley_slider->update(deltaSeconds);
    m_detail_noise_slider->update(deltaSeconds);
    m_cloud_settings_slider->update(deltaSeconds);
    m_perlin_worley_r->update(deltaSeconds);
    m_perlin_worley_g->update(deltaSeconds);
    m_perlin_worley_b->update(deltaSeconds);
    m_perlin_worley_a->update(deltaSeconds);

    m_cloud_data.focal_length = focal_length;
    m_cloud_data.transform = g_theGame->m_camera->GetWorldTransform();
    m_cloud_data_buffer->Update(g_theRenderer->m_deviceContext, &m_cloud_data);

    m_noise_gen_parameters_buffer->Update(g_theRenderer->m_deviceContext, &m_noise_gen_data);
}

void App::Render()
{
	g_theRenderer->SetColorTarget(nullptr, nullptr);
	//g_theRenderer->ClearColor(m_clear_color);
	g_theRenderer->ClearColor(Rgba(126, 192, 238, 255));
	g_theRenderer->ClearDepth();

	int windowWidth = g_theRenderer->m_output->GetWidth();
	int windowHeight = g_theRenderer->m_output->GetHeight();
	g_theRenderer->SetViewport(0, 0, windowWidth, windowHeight);

    if(m_regen_base_noise){
        //render_regen_base_noise();
    }
    
    if(m_regen_detail_noise){
        //render_regen_detail_noise();
    }

    render_clouds();

	g_theRenderer->SetView(g_theGame->m_camera->GetViewTransform());
	g_theRenderer->SetPerspectiveProjection(0.1f, 1000.f, m_viewFov, g_theRenderer->m_output->GetAspectRatio());
    g_theRenderer->SetShaderProgram(nullptr);

	g_theRenderer->DrawCube3d(Vector3::ZERO, 50.0f, AABB2::ZERO_TO_ONE, Rgba::GREY);

	g_theRenderer->DrawWorldAxes();

    if(m_render_sliders_overlap){
        m_base_perlin_worley_slider->render();
        m_base_fbm_worley_slider->render();
        m_detail_noise_slider->render();
        m_cloud_settings_slider->render();
        m_perlin_worley_r->render();
        m_perlin_worley_g->render();
        m_perlin_worley_b->render();
        m_perlin_worley_a->render();
    }

	console_render();

	g_theRenderer->Present();
}

void App::render_regen_base_noise()
{
    g_theRenderer->SetTexture(0, nullptr);
    g_theRenderer->SetTexture(1, nullptr);
    g_theRenderer->SetColorTarget(m_cloud_base, nullptr, false);

    g_theRenderer->EnableBlend(BLEND_ONE, BLEND_ZERO);
    g_theRenderer->EnableDepth(false, false);
	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(Matrix4::IDENTITY);
	g_theRenderer->SetOrthoProjection(AABB2::ZERO_TO_ONE);
    g_theRenderer->SetShaderProgram(m_base_noise_gen_shader);

    g_theRenderer->SetConstantBuffer(4, m_noise_gen_parameters_buffer);

    // draw slices 0-31
    {
        m_noise_gen_data.start_slice = 0;
        g_theRenderer->UpdateConstantBuffer(m_noise_gen_parameters_buffer, &m_noise_gen_data);
        g_theRenderer->DrawQuad2d(AABB2::ZERO_TO_ONE);
    }

    // draw slices 32-63
    {
        m_noise_gen_data.start_slice = 32;
        g_theRenderer->UpdateConstantBuffer(m_noise_gen_parameters_buffer, &m_noise_gen_data);
        g_theRenderer->DrawQuad2d(AABB2::ZERO_TO_ONE);
    }

    // draw slices 64-95
    {
        m_noise_gen_data.start_slice = 64;
        g_theRenderer->UpdateConstantBuffer(m_noise_gen_parameters_buffer, &m_noise_gen_data);
        g_theRenderer->DrawQuad2d(AABB2::ZERO_TO_ONE);
    }

    // draw slices 96-128
    {
        m_noise_gen_data.start_slice = 96;
        g_theRenderer->UpdateConstantBuffer(m_noise_gen_parameters_buffer, &m_noise_gen_data);
        g_theRenderer->DrawQuad2d(AABB2::ZERO_TO_ONE);
    }

    m_regen_base_noise = false;
    g_theRenderer->SetColorTarget();
}

void App::render_regen_detail_noise()
{
    g_theRenderer->SetTexture(0, nullptr);
    g_theRenderer->SetTexture(1, nullptr);
    g_theRenderer->SetColorTarget(m_cloud_detail, nullptr, false);

    g_theRenderer->EnableBlend(BLEND_ONE, BLEND_ZERO);
    g_theRenderer->EnableDepth(false, false);
	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(Matrix4::IDENTITY);
	g_theRenderer->SetOrthoProjection(AABB2::ZERO_TO_ONE);
    g_theRenderer->SetShaderProgram(m_detail_noise_gen_shader);

    g_theRenderer->SetConstantBuffer(4, m_noise_gen_parameters_buffer);

    // draw slices 0-31
    m_noise_gen_data.start_slice = 0;
    g_theRenderer->UpdateConstantBuffer(m_noise_gen_parameters_buffer, &m_noise_gen_data);
    g_theRenderer->DrawQuad2d(AABB2::ZERO_TO_ONE);

    m_regen_detail_noise = false;
    g_theRenderer->SetColorTarget();
}

void App::render_clouds()
{
    g_theRenderer->EnableDepth(true, true);
    g_theRenderer->SetConstantBuffer(4, m_cloud_data_buffer);
    g_theRenderer->SetColorTarget();
    g_theRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
    //g_theRenderer->EnableBlend(BLEND_ONE, BLEND_INV_SRC_ALPHA);
    g_theRenderer->EnableDepth(true, false);
	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(Matrix4::IDENTITY);
	g_theRenderer->SetOrthoProjection(AABB2::ZERO_TO_ONE);
    g_theRenderer->m_deviceContext->SetTexture(0, m_cloud_base);
    g_theRenderer->m_deviceContext->SetTexture(1, m_cloud_detail);
    g_theRenderer->SetShaderProgram(m_clouds_shader);
    g_theRenderer->DrawQuad2d(AABB2::ZERO_TO_ONE);

	// reset blend
    g_theRenderer->EnableBlend(BLEND_ONE, BLEND_ZERO);
}

void App::EndFrame()
{
	if(g_theInputSystem){
		g_theInputSystem->EndFrame();
	}

	BurnLeftOverFrameTime();
}

void App::StepTime()
{
	double timeNowSeconds = get_current_time_seconds();

	m_deltaSeconds = (float)(timeNowSeconds - m_timeOfLastFrameSeconds);
	m_deltaSeconds = Min(m_deltaSeconds, 0.1f); // Cap delta seconds to a tenth of a second

	m_timeOfLastFrameSeconds = timeNowSeconds;
}

void App::BurnLeftOverFrameTime()
{
	bool limitFPS;
	ConfigGetBool(&limitFPS, CONFIG_LIMIT_FPS_NAME);

	if(!limitFPS){
		return;
	}

	int fps;
	ConfigGetInt(&fps, CONFIG_FPS_NAME);

	double minFrameDurationSeconds = 1.0 / (double)fps;
	double frameElapsedTimeSeconds = get_current_time_seconds() - m_timeOfLastFrameSeconds;

	double frameTimeLeft = minFrameDurationSeconds - frameElapsedTimeSeconds;
	if(frameTimeLeft > 0.002f){
		(0.001f);
	}

	frameElapsedTimeSeconds = get_current_time_seconds() - m_timeOfLastFrameSeconds;
	frameTimeLeft = minFrameDurationSeconds - frameElapsedTimeSeconds;
	while(frameTimeLeft > 0.0){
		frameElapsedTimeSeconds = get_current_time_seconds() - m_timeOfLastFrameSeconds;
		frameTimeLeft = minFrameDurationSeconds - frameElapsedTimeSeconds;
	}
}