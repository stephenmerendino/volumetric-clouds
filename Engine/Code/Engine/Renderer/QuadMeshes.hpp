#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"

namespace Meshes
{
	void build_quad_2d(MeshBuilder& mb, 
					   float x_min, 
					   float y_min, 
					   float x_max, 
					   float y_max, 
					   const AABB2& uvs = AABB2::ZERO_TO_ONE,
					   const Rgba& mins_tint = Rgba::WHITE,
					   const Rgba& maxs_tint = Rgba::WHITE);

	void build_quad_2d(MeshBuilder& mb, 
					   const Vector2& mins, 
					   const Vector2& maxs, 
					   const AABB2& uvs = AABB2::ZERO_TO_ONE, 
					   const Rgba& mins_tint = Rgba::WHITE,
					   const Rgba& maxs_tint = Rgba::WHITE);

	void build_quad_2d(MeshBuilder& mb, 
					   const AABB2& bounds, 
					   const AABB2& uvs = AABB2::ZERO_TO_ONE, 
					   const Rgba& mins_tint = Rgba::WHITE,
					   const Rgba& maxs_tint = Rgba::WHITE);

	void build_quad_2d(MeshBuilder& mb, 
					   const Vector2& top_left, 
					   const Vector2& bottom_left, 
					   const Vector2& bottom_right, 
					   const Vector2& top_right, 
					   const AABB2& uvs = AABB2::ZERO_TO_ONE, 
					   const Rgba& mins_tint = Rgba::WHITE,
					   const Rgba& maxs_tint = Rgba::WHITE);

	void build_quad_2d(MeshBuilder& mb,
					   const Vector2& center,
					   const Vector2& right,
					   const Vector2& up,
					   float half_width,
					   float half_height,
					   const AABB2& uvs = AABB2::ZERO_TO_ONE,
					   const Rgba& mins_tint = Rgba::WHITE,
					   const Rgba& maxs_tint = Rgba::WHITE);

	void build_quad_3d(MeshBuilder& mb, 
					   const Vector3& center, 
					   const Vector3& right, 
					   const Vector3& up, 
					   float half_width, 
					   float half_height, 
					   const AABB2& uvs = AABB2::ZERO_TO_ONE, 
					   const Rgba& mins_tint = Rgba::WHITE,
					   const Rgba& maxs_tint = Rgba::WHITE);

	void build_two_sided_quad_3d(MeshBuilder& mb, 
								 const Vector3& center, 
								 const Vector3& right, 
								 const Vector3& up, 
								 float half_width, 
								 float half_height, 
								 const AABB2& texCoords = AABB2::ZERO_TO_ONE, 
								 const Rgba& mins_tint = Rgba::WHITE,
								 const Rgba& maxs_tint = Rgba::WHITE);
}