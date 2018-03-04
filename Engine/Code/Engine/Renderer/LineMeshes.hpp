#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"

namespace Meshes
{
	void build_line_2d(MeshBuilder& mb,
					   const Vector2& start,
					   const Vector2& end,
					   float line_thickness,
					   const Rgba& start_color,
					   const Rgba& end_color);

	void build_line_3d(MeshBuilder& mb,
					   const Vector3& start,
					   const Vector3& end,
					   float line_thickness,
					   const Rgba& start_color,
					   const Rgba& end_color);
}