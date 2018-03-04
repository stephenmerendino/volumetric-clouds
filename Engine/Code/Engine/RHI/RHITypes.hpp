#pragma once

enum RHIOutputMode : unsigned int
{
   RHI_OUTPUT_MODE_WINDOWED,
   RHI_OUTPUT_MODE_BORDERLESS,
   RHI_OUTPUT_MODE_FULLSCREEN_WINDOW,     // Requires Window Optional and ability to resize SwapChain
   RHI_OUTPUT_MODE_FULLSCREEN_DEDICATED,  // Dedicated Fullscreen Mode.  
   NUM_RHI_OUTPUT_MODES
};

enum PrimitiveType : unsigned int
{
   PRIMITIVE_NONE,
   PRIMITIVE_POINTS,
   PRIMITIVE_LINES,
   PRIMITIVE_TRIANGLES,
   NUM_PRIMITIVE_TYPES
};

enum BufferUsage : unsigned int
{
   BUFFERUSAGE_GPU,     // GPU Read/Write, CPU Full Writes        (RenderTargets?)
   BUFFERUSAGE_STATIC,  // GPU Read, Written only at creation     (TEXTURES, STATIC MESHES)
   BUFFERUSAGE_DYNAMIC, // GPU Read, CPU Write (Partial or Full)  (DYNAMIC MESHES/DYNAMIC TEXTURES)
   BUFFERUSAGE_STAGING, // GPU Read/Write, CPU Read/Write         Slowest - image composition/manipulation
   NUM_BUFFERUSAGES
};

enum FilterMode : unsigned int
{
	FILTER_MODE_POINT,
	FILTER_MODE_LINEAR,
	NUM_FILTER_MODES
};

enum CullMode : unsigned int
{
	CULL_FRONT,
	CULL_BACK,
	CULL_NONE
};

enum FillMode : unsigned int
{
	FILL_SOLID,
	FILL_WIREFRAME,
	NUM_FILL_MODES
};

enum DepthTest : unsigned int
{
    DEPTH_TEST_COMPARISON_NEVER,
    DEPTH_TEST_COMPARISON_LESS,
    DEPTH_TEST_COMPARISON_EQUAL,
    DEPTH_TEST_COMPARISON_LESS_EQUAL,
    DEPTH_TEST_COMPARISON_GREATER,
    DEPTH_TEST_COMPARISON_NOT_EQUAL,
    DEPTH_TEST_COMPARISON_GREATER_EQUAL,
    DEPTH_TEST_COMPARISON_ALWAYS
};

enum SamplerComparisonFunc : unsigned int
{
	COMPARISON_NEVER,
    COMPARISON_LESS,
    COMPARISON_EQUAL,
    COMPARISON_LESS_EQUAL,
    COMPARISON_GREATER,
    COMPARISON_NOT_EQUAL,
    COMPARISON_GREATER_EQUAL,
    COMPARISON_ALWAYS
};