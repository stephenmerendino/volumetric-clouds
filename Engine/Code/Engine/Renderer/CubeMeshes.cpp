#include "Engine/Renderer/CubeMeshes.hpp"
#include "Engine/Renderer/QuadMeshes.hpp"

void Meshes::build_cube_3d(MeshBuilder& mb,
						   const Vector3& center,
						   float half_size,
						   const AABB2& uvs,
						   const Rgba& tint)
{
	// Right
	Meshes::build_quad_3d(mb, center + Vector3(half_size, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), half_size, half_size, uvs, tint, tint);
	
	// Left
	Meshes::build_quad_3d(mb, center + Vector3(-half_size, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), half_size, half_size, uvs, tint, tint);

	// Top
	Meshes::build_quad_3d(mb, center + Vector3(0.0f, half_size, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), half_size, half_size, uvs, tint, tint);

	// Bottom
	Meshes::build_quad_3d(mb, center + Vector3(0.0f, -half_size, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), half_size, half_size, uvs, tint, tint);

	// Front
	Meshes::build_quad_3d(mb, center + Vector3(0.0f, 0.0f, half_size), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), half_size, half_size, uvs, tint, tint);

	// Back
	Meshes::build_quad_3d(mb, center + Vector3(0.0f, 0.0f, -half_size), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), half_size, half_size, uvs, tint, tint);
}

void Meshes::build_inverted_cube_3d(MeshBuilder& mb,
									const Vector3& center,
									float half_size,
									const AABB2& uvs,
									const Rgba& tint)
{
	// Right
	Meshes::build_quad_3d(mb, center + Vector3(half_size, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), half_size, half_size, uvs, tint, tint);
	
	// Left
	Meshes::build_quad_3d(mb, center + Vector3(-half_size, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), half_size, half_size, uvs, tint, tint);

	// Top
	Meshes::build_quad_3d(mb, center + Vector3(0.0f, half_size, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), half_size, half_size, uvs, tint, tint);

	// Bottom
	Meshes::build_quad_3d(mb, center + Vector3(0.0f, -half_size, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), half_size, half_size, uvs, tint, tint);

	// Front
	Meshes::build_quad_3d(mb, center + Vector3(0.0f, 0.0f, half_size), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), half_size, half_size, uvs, tint, tint);

	// Back
	Meshes::build_quad_3d(mb, center + Vector3(0.0f, 0.0f, -half_size), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), half_size, half_size, uvs, tint, tint);
}