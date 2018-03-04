#pragma once

class ConsoleDisplay;

#include "Engine/Core/Rgba.hpp"

bool ConfigSystemStartup(const char* filename, const char* commandLine = "");
void ConfigSystemShutdown();

void ConfigSet(const char* name, const char* value);
void ConfigSet(const char* name, const bool value);
void ConfigSet(const char* name, const int value);
void ConfigSet(const char* name, const float value);
void ConfigSet(const char* name, const Rgba& rgba);

bool ConfigGetString(char** out_value, const char* name);
bool ConfigGetBool(bool* out_value, const char* name);
bool ConfigGetInt(int* out_value, const char* name);
bool ConfigGetFloat(float* out_value, const char* name);
bool ConfigGetRgba(Rgba* out_value, const char* name);

bool IsConfigSet(const char* name);
void ConfigUnset(const char* name);

void ConfigList();

bool ConfigLoadFile(const char* filename);
bool ConfigSystemSaveToFile(const char* filename);