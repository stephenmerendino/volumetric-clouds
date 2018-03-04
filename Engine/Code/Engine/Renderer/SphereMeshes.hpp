#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"

namespace Meshes
{
	void build_uv_sphere(MeshBuilder& mb, const Vector3& center, float radius, const Rgba& color = Rgba::WHITE, unsigned int slices = 64);
	//void build_ico_sphere(MeshBuilder& mb, ......);
}