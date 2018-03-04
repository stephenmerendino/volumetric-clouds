#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"

namespace Meshes
{
	void build_circle_2d(MeshBuilder& mb, 
						 const Vector2& center, 
						 float radius, 
						 float margin, 
						 float padding, 
						 const Rgba& border_color, 
						 const Rgba& fill_color, 
						 int num_sides = 64);
}