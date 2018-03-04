#include "Engine/Renderer/CircleMeshes.hpp"
#include "Engine/Renderer/QuadMeshes.hpp"

void Meshes::build_circle_2d(MeshBuilder& mb,
							 const Vector2& center,
							 float radius,
							 float margin,
							 float padding,
							 const Rgba& border_color,
							 const Rgba& fill_color, 
							 int num_sides)
{
	float degrees_step = 360.f / (float)num_sides;

	for(float degrees = 0.0f; degrees < 360.f; degrees += degrees_step){
		float x_now = CosDegrees(degrees) * radius;
		float y_now = SinDegrees(degrees) * radius;

		float x_next = CosDegrees(degrees + degrees_step) * radius;
		float y_next = SinDegrees(degrees + degrees_step) * radius;

		Vector2 cur_pos = center + Vector2(x_now, y_now);
		Vector2 next_pos = center + Vector2(x_next, y_next);

		Vector2 dir_from_center_now = Vector2(x_now, y_now).Normalized();
		Vector2 dir_from_center_next = Vector2(x_next, y_next).Normalized();

		Vector2 top_right = cur_pos + (dir_from_center_now * margin);
		Vector2 bottom_right = cur_pos - (dir_from_center_now * padding);
		Vector2 bottom_left = next_pos - (dir_from_center_next * padding);
		Vector2 top_left = next_pos + (dir_from_center_next * margin);

		// outside
		build_quad_2d(mb, top_left, bottom_left, bottom_right, top_right, AABB2::ZERO_TO_ONE, border_color, border_color);

		// inside
		build_quad_2d(mb, bottom_left, center, center, bottom_right, AABB2::ZERO_TO_ONE, fill_color, fill_color);
	}
}