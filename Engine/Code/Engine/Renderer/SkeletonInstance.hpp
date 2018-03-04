#pragma once

#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Renderer/Pose.hpp"
#include "Engine/Renderer/Motion.hpp"
#include "Engine/RHI/StructuredBuffer.hpp"

class SkeletonInstance
{
public:
	Skeleton* m_skeleton; // skeleton we're applying poses to.  Used for heirachy information.
	Pose m_current_pose;  // my current skeletons pose.

	StructuredBuffer* m_structured_buffer;

public:
	SkeletonInstance(Skeleton* skeleton);

	// Excercise:  How do we evaluate a global transform for this joint if the pose only contains 
	// local transforms?
	// Hint:  You will need "get_joint_parent" below.
	Matrix4 get_joint_global_transform(unsigned int joint_index) const;
	Matrix4 get_joint_local_transform(unsigned int joint_index) const;
	unsigned int get_joint_parent_index(unsigned int joint_index) const;

	void apply_motion(const Motion* motion, const float time);

	void draw() const;
};