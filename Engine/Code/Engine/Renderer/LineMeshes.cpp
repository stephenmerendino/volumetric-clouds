#include "Engine/Renderer/LineMeshes.hpp"
#include "Engine/Renderer/QuadMeshes.hpp"

#include "Engine/Core/Common.hpp"

void Meshes::build_line_2d(MeshBuilder& mb,
						   const Vector2& start,
						   const Vector2& end,
						   float line_thickness, 
						   const Rgba& start_color, 
						   const Rgba& end_color)
{
	Vector2 displacement = end - start;

	Vector2 up = displacement.Normalized();

	Vector2 right = up;
	right.Rotate90Degrees();

	Vector2 center = start + (displacement / 2.0f);

	float length = displacement.CalcLength();

	Meshes::build_quad_2d(mb, center, right, up, line_thickness * 0.5f, length * 0.5f, AABB2::ZERO_TO_ONE, start_color, end_color);
}

void Meshes::build_line_3d(MeshBuilder& mb,
						   const Vector3& start,
						   const Vector3& end,
						   float line_thickness,
						   const Rgba& start_color,
						   const Rgba& end_color)
{
	UNUSED(line_thickness);

	mb.begin(PRIMITIVE_LINES, false);

	mb.set_color(start_color);
	mb.add_vertex(start);

	mb.set_color(end_color);
	mb.add_vertex(end);

	mb.end();
}