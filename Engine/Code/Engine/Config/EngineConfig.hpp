#pragma once

#include "Engine/Core/Rgba.hpp"

static const int DEFAULT_WINDOW_WIDTH = 1600;
static const int DEFAULT_WINDOW_HEIGHT = 900;
static const char* DEFAULT_WINDOW_NAME = "Phendino Engine";
static const Rgba DEFAULT_CLEAR_COLOR = Rgba(0, 255, 0, 255);

static const char* FONT_DIRECTORY = "Data/Fonts/";
static const char* ENGINE_FONT_FILE = "consolas32.fnt";
//static const char* ENGINE_FONT_FILE = "calibri32.fnt";

static const char* DX_VERTEX_SHADER_TARGET = "vs_5_0";
static const char* DX_GEOMETRY_SHADER_TARGET = "gs_5_0";
static const char* DX_FRAGMENT_SHADER_TARGET = "ps_5_0";
static const char* DX_COMPUTE_SHADER_TARGET = "cs_5_0";

static const char* DEFAULT_VERTEX_SHADER_ENTRY_POINT = "VertexFunction";
static const char* DEFAULT_GEOMETRY_SHADER_ENTRY_POINT = "GeometryFunction";
static const char* DEFAULT_FRAGMENT_SHADER_ENTRY_POINT = "FragmentFunction";
static const char* DEFAULT_COMPUTE_SHADER_ENTRY_POINT = "ComputeFunction";

static const Rgba CONSOLE_MAIN_BG_COLOR = Rgba(0, 0, 0, 220);
static const Rgba CONSOLE_ACCENT_BG_COLOR = Rgba(0, 255, 0, 220);

#define RCS_PORT 1313