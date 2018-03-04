#pragma once

#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/event.h"
#include <string>

#pragma warning( disable : 4100 )   // Disable "unreferenced formal parameter"



//---------------------------------------------------------------------------------
class ConsoleArgs
{
public:
	std::string m_raw_args;

public:
	ConsoleArgs();
	ConsoleArgs(const std::string& raw_args);

	std::string next_string_arg();
	unsigned int next_uint_arg();
	int next_int_arg();
	float next_float_arg();
	bool next_bool_arg();

    std::string get_remaining_args_as_string();

	bool is_at_end();

private:
	unsigned int m_cursor;

	void advance_to_next_token();
};

typedef void (*ConsoleFunction)(ConsoleArgs& args);



//---------------------------------------------------------------------------------
class CommandSelfRegister
{
public:
	CommandSelfRegister(const std::string& command_name, ConsoleFunction func, const std::string& desc);
};

#define COMMAND(command_name, desc) \
	static void AutoCommand_##command_name##(ConsoleArgs&); \
	static CommandSelfRegister g_command_register_##command_name##(#command_name, AutoCommand_##command_name##, desc); \
	static void AutoCommand_##command_name##(ConsoleArgs& args)



//---------------------------------------------------------------------------------
void console_init(Font& font);
void console_shutdown();

void console_update(float delta_seconds);

void console_set_font(const Font& font);
void console_render();

void console_show();
void console_hide();
void console_toggle_visibility();

void console_clear();

bool is_console_open();

void console_register_char_key_down(char c);
void console_register_non_char_key_down(unsigned char uc);
void console_register_non_char_key_up(unsigned char uc);

void console_printf(const Rgba& color, const char* format, ...);
void console_printf(const Rgba& color, const std::string& text);

void console_info(const char* format, ...);
void console_info(const std::string& text);
void console_warning(const char* format, ...);
void console_warning(const std::string& text);
void console_error(const char* format, ...);
void console_error(const std::string& text);
void console_success(const char* format, ...);
void console_success(const std::string& text);

void console_register_command(const std::string& command_name, ConsoleFunction func, const std::string& desc = "");
void console_run_command(const std::string& command_name);
void console_run_command_and_args(const std::string& command_and_args);

typedef void(print_cb)(void*, const std::string&);
void console_register_to_print_event(void* user_arg, print_cb cb);
void console_unregister_to_print_event(void* user_arg, print_cb cb);