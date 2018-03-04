#include "Engine/Renderer/SkeletonInstance.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Engine.hpp"

SkeletonInstance::SkeletonInstance(Skeleton* skeleton)
	:m_skeleton(skeleton)
	,m_structured_buffer(nullptr)
{
}

// Excercise:  How do we evaluate a global transform for this joint if the pose only contains 
// local transforms?
// Hint:  You will need "get_joint_parent" below.
Matrix4 SkeletonInstance::get_joint_global_transform(unsigned int joint_index) const
{
	Matrix4 global_transform = m_current_pose.m_local_transforms[joint_index];

	unsigned int current_joint = joint_index;
	while(m_skeleton->joint_has_parent(current_joint)){
		current_joint = m_skeleton->get_joint_parent_index(current_joint);
		global_transform *= m_current_pose.m_local_transforms[current_joint];
	}

	return global_transform;
}

Matrix4 SkeletonInstance::get_joint_local_transform(unsigned int joint_index) const
{
	return m_current_pose.m_local_transforms[joint_index];
}

unsigned int SkeletonInstance::get_joint_parent_index(unsigned int joint_index) const
{
	return m_skeleton->get_joint_parent_index(joint_index);
}

void SkeletonInstance::apply_motion(const Motion* motion, const float time)
{
	motion->evaluate(&m_current_pose, time);

	std::vector<Matrix4> skinning_transforms;
	skinning_transforms.resize(m_skeleton->m_transform_hierarchy.m_transforms.size());

	for(size_t i = 0; i < skinning_transforms.size(); ++i){
		const Matrix4& bind_pose_joint = m_skeleton->m_transform_hierarchy.get_transform(i);
		const Matrix4& current_pose_joint = get_joint_global_transform(i);
		skinning_transforms[i] = bind_pose_joint.get_inverse() * current_pose_joint;
	}

	if(!m_structured_buffer){
		m_structured_buffer = new StructuredBuffer(g_theRenderer->m_device, skinning_transforms.data(), sizeof(Matrix4), skinning_transforms.size());
	}
	else{
		g_theRenderer->UpdateStructuredBuffer(m_structured_buffer, skinning_transforms.data());
	}
}

void SkeletonInstance::draw() const
{
	g_theRenderer->draw_skeleton_instance(this);
}