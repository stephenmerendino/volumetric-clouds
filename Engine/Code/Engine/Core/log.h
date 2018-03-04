#pragma once

#include "Engine/Core/Rgba.hpp"

#define MAX_MESSAGE_SIZE 512

void log_init(const char* log_directory);
void log_shutdown();
void log_flush();

void log_printf(const char* format, ...);

void log_tagged_printf(const char* tag, const char* format, ...);
void log_warningf(const char* format, ...);
void log_errorf(const char* format, ...);

void log_printf_with_callstack(const char* format, ...);
void log_tagged_printf_with_callstack(const char* tag, const char* format, ...);

void log_disable_tag(const char* tag);
void log_enable_tag(const char* tag);

void log_disable_all_tags();
void log_enable_all_tags();

void log_set_console_tag_color(const char* tag, const Rgba& color);