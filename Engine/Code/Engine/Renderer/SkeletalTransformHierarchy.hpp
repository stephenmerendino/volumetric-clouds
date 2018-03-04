#pragma once

#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/Quaternion.hpp"
#include <vector>

#define INVALID_TRANSFORM_INDEX ((unsigned int)-1)

/* TODO: Implement quaternion
struct transform_srt_t 
{
	Vector3 scale;
	Quaternion rotation;
	Vector3 position;
};
*/

class SkeletalTransformHierarchy
{
public:
	std::vector<Matrix4> m_transforms;
	std::vector<std::string> m_transform_names;
	std::vector<unsigned int> m_parent_indexes;

public:
	SkeletalTransformHierarchy();
	SkeletalTransformHierarchy(const Matrix4& transform);

	SkeletalTransformHierarchy(const SkeletalTransformHierarchy& other);
	SkeletalTransformHierarchy& operator=(const SkeletalTransformHierarchy& other);

	void clear();

	void add_transform(const char* name, 
					   const char* parent_name, 
					   const Matrix4& transform);

	unsigned int get_transform_count() const;
	unsigned int get_transform_index(const char* name) const;

	std::string get_transform_name(unsigned int joint_index) const;

	Matrix4 get_transform(unsigned int transform_index) const;
	Matrix4 get_transform(const char* name) const;

	bool transform_has_parent(unsigned int transform_index) const;
	bool transform_has_parent(const char* name) const;

	unsigned int get_transform_parent_index(unsigned int transform_index) const;
	unsigned int get_transform_parent_index(const char* name) const;

	Matrix4 get_parent_transform(unsigned int transform_index) const;
	Matrix4 get_parent_transform(const char* name) const;
};