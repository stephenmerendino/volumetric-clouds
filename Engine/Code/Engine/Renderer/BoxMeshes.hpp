#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"

namespace Meshes
{
	void build_box_2d(MeshBuilder& mb, 
					  const AABB2& bounds, 
					  float margin, 
					  float padding,
					  const Rgba& edge_color, 
					  const Rgba& fill_color);
}