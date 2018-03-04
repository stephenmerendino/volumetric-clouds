#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Math/Matrix4.hpp"

namespace Meshes
{
	void build_scene_axes(MeshBuilder& mb, const Matrix4& local_to_world, float axis_length = 100.0f);
	void build_scene_axes(MeshBuilder& mb, float axis_length = 100.f);

	void build_scene_grid_xz(MeshBuilder& mb, 
							 float grid_size = 200.f, 
							 float major_unit_stride = 1.0f, 
							 float minor_unit_stride = 0.2f, 
							 const Rgba& major_unit_color = Rgba(255, 255, 255, 80), 
							 const Rgba& minor_unit_color = Rgba(255, 255, 255, 8));
}