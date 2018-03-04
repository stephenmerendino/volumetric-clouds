#include "Engine/Renderer/QuadMeshes.hpp"

void Meshes::build_quad_2d(MeshBuilder& mb,
						   float x_min,
						   float y_min,
						   float x_max,
						   float y_max,
						   const AABB2& uvs,
						   const Rgba& mins_tint,
						   const Rgba& maxs_tint)
{
	Vector3 bottom_left(x_min, y_min, 0.0f);
	Vector3 top_left(x_min, y_max, 0.0f);
	Vector3 top_right(x_max, y_max, 0.0f);
	Vector3 bottom_right(x_max, y_min, 0.0f);

	mb.begin(PRIMITIVE_TRIANGLES, true);

	mb.set_color(mins_tint);
	mb.set_uv(uvs.CalcTopLeft());
	size_t index = mb.add_vertex(bottom_left);

	mb.set_color(maxs_tint);
	mb.set_uv(uvs.mins);
	mb.add_vertex(top_left);

	mb.set_color(maxs_tint);
	mb.set_uv(uvs.CalcBottomRight());
	mb.add_vertex(top_right);

	mb.set_color(mins_tint);
	mb.set_uv(uvs.maxs);
	mb.add_vertex(bottom_right);

	unsigned int indexes[6] = {index, index + 2, index + 1, index, index + 3, index + 2};
	mb.add_indexes(indexes, 6);

	mb.end();
}

void Meshes::build_quad_2d(MeshBuilder& mb,
						   const Vector2& mins,
						   const Vector2& maxs,
						   const AABB2& uvs,
						   const Rgba& mins_tint,
						   const Rgba& maxs_tint)
{
	build_quad_2d(mb, mins.x, mins.y, maxs.x, maxs.y, uvs, mins_tint, maxs_tint);
}

void Meshes::build_quad_2d(MeshBuilder& mb,
						   const AABB2& bounds,
						   const AABB2& uvs,
						   const Rgba& mins_tint,
						   const Rgba& maxs_tint)
{
	build_quad_2d(mb, bounds.mins, bounds.maxs, uvs, mins_tint, maxs_tint);
}

void Meshes::build_quad_2d(MeshBuilder& mb,
						   const Vector2& top_left,
						   const Vector2& bottom_left,
						   const Vector2& bottom_right,
						   const Vector2& top_right,
						   const AABB2& uvs,
						   const Rgba& mins_tint,
						   const Rgba& maxs_tint)
{
	mb.begin(PRIMITIVE_TRIANGLES, true);

	mb.set_color(mins_tint);
	mb.set_uv(uvs.CalcTopLeft());
	size_t index = mb.add_vertex(bottom_left);

	mb.set_color(maxs_tint);
	mb.set_uv(uvs.mins);
	mb.add_vertex(top_left);

	mb.set_color(maxs_tint);
	mb.set_uv(uvs.CalcBottomRight());
	mb.add_vertex(top_right);

	mb.set_color(mins_tint);
	mb.set_uv(uvs.maxs);
	mb.add_vertex(bottom_right);

	unsigned int indexes[6] = {index, index + 2, index + 1, index, index + 3, index + 2};
	mb.add_indexes(indexes, 6);

	mb.end();
}

void Meshes::build_quad_2d(MeshBuilder& mb,
						   const Vector2& center,
						   const Vector2& right,
						   const Vector2& up,
						   float half_width,
						   float half_height,
						   const AABB2& uvs,
						   const Rgba& mins_tint,
						   const Rgba& maxs_tint)
{
	build_quad_3d(mb, Vector3(center, 0.f), Vector3(right, 0.f), Vector3(up, 0.f), half_width, half_height, uvs, mins_tint, maxs_tint);
}

void Meshes::build_quad_3d(MeshBuilder& mb,
						   const Vector3& center,
						   const Vector3& right,
						   const Vector3& up,
						   float half_width,
						   float half_height,
						   const AABB2& uvs,
						   const Rgba& mins_tint,
						   const Rgba& maxs_tint)
{
	// Make sure the dir's are normalized
	Vector3 right_norm = right.Normalized();
	Vector3 up_norm = up.Normalized();

	// Vertex Positions
	Vector3 bottom_left = center + (right_norm * -half_width) + (up_norm * -half_height);
	Vector3 top_left = center + (right_norm * -half_width) + (up_norm * half_height);
	Vector3 top_right = center + (right_norm * half_width) + (up_norm * half_height);
	Vector3 bottom_right = center + (right_norm * half_width) + (up_norm * -half_height);

	// TBN
	Vector3 normal = CrossProduct(up_norm, right_norm);
	Vector3 tangent = right_norm;
	Vector3 bitangent = up_norm;

	mb.begin(PRIMITIVE_TRIANGLES, true);

	mb.set_normal(normal);
	mb.set_tangent(tangent);
	mb.set_bitangent(bitangent);

	mb.set_color(mins_tint);
	mb.set_uv(uvs.CalcTopLeft());
	size_t index = mb.add_vertex(bottom_left);

	mb.set_color(maxs_tint);
	mb.set_uv(uvs.mins);
	mb.add_vertex(top_left);

	mb.set_color(maxs_tint);
	mb.set_uv(uvs.CalcBottomRight());
	mb.add_vertex(top_right);

	mb.set_color(mins_tint);
	mb.set_uv(uvs.maxs);
	mb.add_vertex(bottom_right);

	unsigned int indexes[6] = {index, index + 2, index + 1, index, index + 3, index + 2};
	mb.add_indexes(indexes, 6);

	mb.end();
}

void Meshes::build_two_sided_quad_3d(MeshBuilder& mb,
									 const Vector3& center,
									 const Vector3& right,
									 const Vector3& up,
									 float half_width,
									 float half_height,
									 const AABB2& texCoords,
									 const Rgba& mins_tint,
									 const Rgba& maxs_tint)
{
	Meshes::build_quad_3d(mb, center, right, up, half_width, half_height, texCoords, mins_tint, maxs_tint);
	Meshes::build_quad_3d(mb, center, -right, up, half_width, half_height, texCoords, mins_tint, maxs_tint);
}