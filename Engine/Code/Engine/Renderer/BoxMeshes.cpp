#include "Engine/Renderer/BoxMeshes.hpp"
#include "Engine/Renderer/QuadMeshes.hpp"

void Meshes::build_box_2d(MeshBuilder& mb, 
						  const AABB2& bounds, 
						  float margin, 
						  float padding, 
						  const Rgba& edge_color, 
						  const Rgba& fill_color)
{
	Vector2 bounds_top_left = bounds.CalcTopLeft();
	Vector2 bounds_top_right = bounds.maxs;
	Vector2 bounds_bottom_right = bounds.CalcBottomRight();
	Vector2 bounds_bottom_left = bounds.mins;

	// Inside Quad
	Meshes::build_quad_2d(mb, bounds_bottom_left, bounds_top_right, AABB2::ZERO_TO_ONE, fill_color, fill_color);

	// Top Quad
	Vector2 top_quad_top_right = bounds_top_right + Vector2(margin, margin);
	Vector2 top_quad_bottom_left = bounds_top_left + Vector2(-margin, -padding);
	Meshes::build_quad_2d(mb, top_quad_bottom_left, top_quad_top_right, AABB2::ZERO_TO_ONE, edge_color, edge_color);

	// Bottom Quad
	Vector2 bottom_quad_top_right = bounds_bottom_right + Vector2(margin, padding);
	Vector2 bottom_quad_bottom_left = bounds_bottom_left + Vector2(-margin, -margin);
	Meshes::build_quad_2d(mb, bottom_quad_bottom_left, bottom_quad_top_right, AABB2::ZERO_TO_ONE, edge_color, edge_color);

	// Left Quad
	Vector2 left_quad_top_right = bounds_top_left + Vector2(padding, 0.0f);
	Vector2 left_quad_bottom_left = bounds_bottom_left + Vector2(-margin, 0.0f);
	Meshes::build_quad_2d(mb, left_quad_bottom_left, left_quad_top_right, AABB2::ZERO_TO_ONE, edge_color, edge_color);

	// Right Quad
	Vector2 right_quad_top_right = bounds_top_right + Vector2(margin, 0.0f);
	Vector2 right_quad_bottom_left = bounds_bottom_right + Vector2(-padding, 0.0f);
	Meshes::build_quad_2d(mb, right_quad_bottom_left, right_quad_top_right, AABB2::ZERO_TO_ONE, edge_color, edge_color);
}