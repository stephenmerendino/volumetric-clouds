#include "Engine/engine.hpp"
#include "Engine/Config/EngineConfig.hpp"
#include "Engine/RHI/RHI.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Core/Console.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Config.hpp"
#include "Engine/Core/log.h"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/job.h"
#include "Engine/Net/net.hpp"
#include "Engine/Net/remote_command_service.hpp"
#include "Engine/Net/Object/net_object_system.hpp"
#include "Engine/Profile/profiler.h"
#include "Engine/Profile/mem_tracker.h"
#include "Engine/Profile/callstack.h"
#include "Engine/Profile/profiler_visualizer.h"
#include "Engine/Config/build_config.h"
#include "Engine/Core/process.hpp"
#include "Engine/Input/midi.h"
#include <time.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

SimpleRenderer* g_theRenderer = nullptr;
AudioSystem* g_theAudioSystem = nullptr;
InputSystem* g_theInputSystem = nullptr;

static engine_startup_t s_startup_params;
static Font* s_engine_font = nullptr;
static double s_current_frame_time = 0;

void engine_init(engine_startup_t params)
{
	s_startup_params = params;

    time_init();

	// init rng
	srand((unsigned int)time(NULL) + (unsigned int)get_current_perf_counter());

    mem_tracker_init();

    job_system_init();

    log_init(LOG_FILE_DIRECTORY);

    profiler_init();

	// init config system
	ASSERT_OR_DIE(ConfigSystemStartup(s_startup_params.config_filename, s_startup_params.command_line), "ConfigSystemStartup failed.\n");


	// init window and rhi sub system
	RegisterGameWindowClass();
	RHISystemStartup();

    net_system_init();
    net_object_system_init();

	// init major engine systems
	g_theInputSystem = new InputSystem();
    midi_init();

	g_theAudioSystem = new AudioSystem();
	g_theRenderer = new SimpleRenderer();

	// check if config specifies a width & height
	int width = DEFAULT_WINDOW_WIDTH;
	ConfigGetInt(&width, params.config_key_window_width);

	int height = DEFAULT_WINDOW_HEIGHT;
	ConfigGetInt(&height, params.config_key_window_height);

	// setup the renderering system
	g_theRenderer->Setup(width, height);
	g_theRenderer->m_output->m_window->SetCustomWindowsMessageCallback(params.windows_message_cb);
	g_theRenderer->m_output->CenterDisplay();

	// init window title
	char* title;
	if(ConfigGetString(&title, params.config_key_window_title)){
		g_theRenderer->m_output->m_window->SetTitle(title);
	} else{
		g_theRenderer->m_output->m_window->SetTitle(DEFAULT_WINDOW_NAME);
	}

	// init developer console
	s_engine_font = g_theRenderer->m_device->CreateFontFromFile(ENGINE_FONT_FILE);
	console_init(*s_engine_font);

    RemoteCommandService::init();
}

void engine_shutdown()
{
    RemoteCommandService::shutdown();


    SAFE_DELETE(s_engine_font);

	// shutdown console
	console_shutdown();

	// shutdown major engine systems
	SAFE_DELETE(g_theRenderer);
	SAFE_DELETE(g_theAudioSystem);
	SAFE_DELETE(g_theInputSystem);
    midi_shutdown();

    net_object_system_shutdown();
    net_system_shutdown();

	// shutdown rhi and window sub systems
	RHISystemShutdown();
	UnregisterGameWindowClass();

	// save out config to file if needed
	bool autosave_config;
	ConfigGetBool(&autosave_config, s_startup_params.config_key_config_autosave);

	if(autosave_config){
		char* config_autosave_filepath;
		ConfigGetString(&config_autosave_filepath, s_startup_params.config_key_config_autosave_filepath);
		ConfigSystemSaveToFile(config_autosave_filepath);
	}

	// shutdown config system
	ConfigSystemShutdown();

    profiler_shutdown();

    mem_log_live_allocs();

    log_shutdown();

    job_system_shutdown();

    mem_tracker_shutdown();
}

void engine_tick(float ds)
{
    s_current_frame_time += ds;
    mem_tracker_tick();
    job_system_main_step();
	console_update(ds);
    RemoteCommandService::get_instance()->update(ds);
    net_object_system_tick();
	g_theInputSystem->BeginFrame();
	g_theRenderer->m_output->ProcessMessages();
	g_theRenderer->Update(ds);
}

void engine_render()
{
    profiler_render();
	console_render();
    RemoteCommandService::get_instance()->render();
}

double engine_get_current_frame_time_seconds()
{
    return s_current_frame_time;
}

void sleep(double seconds)
{
	unsigned long milliseconds = (unsigned long)(seconds * 1000.0f);
	Sleep(milliseconds);
}

const Font& get_engine_font()
{
	return *s_engine_font;
}

COMMAND(launch, "[num_new_apps:int] Launches n new instances of this app")
{
    std::string app_name = get_app_name();
    unsigned int num_to_launch = args.next_uint_arg();

    for(unsigned int i = 0; i < num_to_launch; i++){
        bool launched = launch_program(app_name.c_str(), nullptr);
        if(!launched){
            log_warningf("Failed to launch app [%i instance of %s]", i, app_name.c_str());
            return;
        }else{
            log_printf("Launched");
        }
    }
}