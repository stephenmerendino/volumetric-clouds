#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"

namespace Meshes
{
	void build_cube_3d(MeshBuilder& mb,
					   const Vector3& center,
					   float half_size,
					   const AABB2& uvs = AABB2::ZERO_TO_ONE,
					   const Rgba& tint = Rgba::WHITE);

	void build_inverted_cube_3d(MeshBuilder& mb,
								const Vector3& center,
								float half_size,
								const AABB2& uvs = AABB2::ZERO_TO_ONE,
								const Rgba& tint = Rgba::WHITE);
}