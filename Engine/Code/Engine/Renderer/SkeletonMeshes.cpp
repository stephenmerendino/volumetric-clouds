#include "Engine/Renderer/SkeletonMeshes.hpp"
#include "Engine/Renderer/LineMeshes.hpp"
#include "Engine/Renderer/SceneMeshes.hpp"

static void draw_bone(MeshBuilder& mb,
					  const Matrix4& bone_start_global_transform,
					  const Matrix4& bone_end_global_transform)
{
	Vector3 start_pos = bone_start_global_transform.get_translation().xyz;
	Vector3 end_pos = bone_end_global_transform.get_translation().xyz;

	Vector3 displacement = end_pos - start_pos;
	Vector3 dir = displacement.Normalized();

	float cross_length = displacement.CalcLength() * 0.1f;
	Vector3 cross_section_pos = start_pos + (displacement * 0.2f);

	Vector3 i = bone_start_global_transform.get_i_basis().xyz;
	Vector3 j = bone_start_global_transform.get_j_basis().xyz;
	Vector3 k = bone_start_global_transform.get_k_basis().xyz;

	Meshes::build_line_3d(mb, start_pos, end_pos, 1.0f, Rgba::WHITE, Rgba::WHITE);

	float basis_length = cross_length * 0.5f;

	Meshes::build_line_3d(mb, end_pos, end_pos + (i * basis_length), 1.0f, Rgba::RED, Rgba::RED);
	Meshes::build_line_3d(mb, end_pos, end_pos + (j * basis_length), 1.0f, Rgba::GREEN, Rgba::GREEN);
	Meshes::build_line_3d(mb, end_pos, end_pos + (k * basis_length), 1.0f, Rgba::BLUE, Rgba::BLUE);

	Meshes::build_line_3d(mb, start_pos, cross_section_pos + (i * cross_length), 1.0f, Rgba::RED, Rgba::RED);
	Meshes::build_line_3d(mb, cross_section_pos + (i * cross_length), end_pos, 1.0f, Rgba::RED, Rgba::RED);

	Meshes::build_line_3d(mb, start_pos, cross_section_pos + (k * cross_length), 1.0f, Rgba::BLUE, Rgba::BLUE);
	Meshes::build_line_3d(mb, cross_section_pos + (k * cross_length), end_pos, 1.0f, Rgba::BLUE, Rgba::BLUE);

	Meshes::build_line_3d(mb, start_pos, cross_section_pos + (-i * cross_length), 1.0f, Rgba::WHITE, Rgba::WHITE);
	Meshes::build_line_3d(mb, cross_section_pos + (-i * cross_length), end_pos, 1.0f, Rgba::WHITE, Rgba::WHITE);

	Meshes::build_line_3d(mb, start_pos, cross_section_pos + (-k * cross_length), 1.0f, Rgba::WHITE, Rgba::WHITE);
	Meshes::build_line_3d(mb, cross_section_pos + (-k * cross_length), end_pos, 1.0f, Rgba::WHITE, Rgba::WHITE);
}

void Meshes::build_skeleton(MeshBuilder& mb, const Skeleton* skeleton)
{
	for(unsigned int joint_count = 0; joint_count < skeleton->get_joint_count(); ++joint_count){
		if(skeleton->joint_has_parent(joint_count)){
			Matrix4 transform = skeleton->get_joint_transform(joint_count);
			Matrix4 parent_transform = skeleton->get_joint_parent_transform(joint_count);

			draw_bone(mb, parent_transform, transform);
		}
	}
}

void Meshes::build_skeleton_instance(MeshBuilder& mb, const SkeletonInstance* skeleton_instance)
{
	for(unsigned int joint_count = 0; joint_count < skeleton_instance->m_skeleton->get_joint_count(); ++joint_count){
		if(skeleton_instance->m_skeleton->joint_has_parent(joint_count)){
			Matrix4 transform = skeleton_instance->get_joint_global_transform(joint_count);

			unsigned int parent_index = skeleton_instance->get_joint_parent_index(joint_count);
			Matrix4 parent_transform = skeleton_instance->get_joint_global_transform(parent_index);

			draw_bone(mb, parent_transform, transform);
		}
	}
}