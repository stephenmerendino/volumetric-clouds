#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"

typedef float (*HeightFunc)(float x, float z);

namespace Meshes
{
	void build_surface_patch(MeshBuilder& mb, 
							 HeightFunc heightFunc, 
							 float width, 
							 float length, 
							 unsigned int slices, 
							 float uv_density = 1.0f);
}