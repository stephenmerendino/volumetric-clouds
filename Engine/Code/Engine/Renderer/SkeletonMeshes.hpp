#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Renderer/SkeletonInstance.hpp"

namespace Meshes
{
	void build_skeleton(MeshBuilder& mb, const Skeleton* skeleton);
	void build_skeleton_instance(MeshBuilder& mb, const SkeletonInstance* skeleton_instance);
}