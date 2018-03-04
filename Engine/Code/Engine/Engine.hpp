#pragma once

#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Window.hpp"

class Font;

extern SimpleRenderer* g_theRenderer;
extern AudioSystem* g_theAudioSystem;
extern InputSystem* g_theInputSystem;

struct engine_startup_t
{
	const char*	command_line;
	const char*	config_filename;
	const char*	config_key_config_autosave;
	const char*	config_key_config_autosave_filepath;
	const char*	config_key_window_width;
	const char*	config_key_window_height;
	const char*	config_key_window_title;
	WindowsMessageCallback windows_message_cb;
};

void engine_init(engine_startup_t params);
void engine_shutdown();
void engine_tick(float ds);
void engine_render();
double engine_get_current_frame_time_seconds();

void sleep(double seconds);
const Font& get_engine_font();