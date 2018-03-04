#include "Engine/Renderer/SkeletalTransformHierarchy.hpp"

SkeletalTransformHierarchy::SkeletalTransformHierarchy()
{
}

SkeletalTransformHierarchy::SkeletalTransformHierarchy(const Matrix4& transform)
{
	add_transform("root", nullptr, transform);
}

SkeletalTransformHierarchy::SkeletalTransformHierarchy(const SkeletalTransformHierarchy& other)
{
	*this = other;
}

SkeletalTransformHierarchy& SkeletalTransformHierarchy::operator=(const SkeletalTransformHierarchy& other)
{
	m_transforms = other.m_transforms;
	m_transform_names = other.m_transform_names;
	m_parent_indexes = other.m_parent_indexes;

	return *this;
}

void SkeletalTransformHierarchy::clear()
{
	m_transforms.clear();
	m_transform_names.clear();
	m_parent_indexes.clear();
}

void SkeletalTransformHierarchy::add_transform(const char* name,
							  const char* parent_name,
							  const Matrix4& transform)
{
	m_transforms.push_back(transform);
	m_transform_names.push_back(name);

	unsigned int parent_index = get_transform_index(parent_name);
	m_parent_indexes.push_back(parent_index);
}

unsigned int SkeletalTransformHierarchy::get_transform_count() const
{
	return m_transforms.size();
}

unsigned int SkeletalTransformHierarchy::get_transform_index(const char* name) const
{
	if(name == nullptr){
		return INVALID_TRANSFORM_INDEX;
	}

	for(size_t transform_index = 0; transform_index < m_transforms.size(); ++transform_index){
		if(m_transform_names[transform_index] == name){
			return transform_index;
		}
	}

	return INVALID_TRANSFORM_INDEX;
}

std::string SkeletalTransformHierarchy::get_transform_name(unsigned int joint_index) const
{
	return m_transform_names[joint_index];
}

Matrix4 SkeletalTransformHierarchy::get_transform(unsigned int transform_index) const
{
	return m_transforms[transform_index];
}

Matrix4 SkeletalTransformHierarchy::get_transform(const char* name) const
{
	int transform_index = get_transform_index(name);
	return get_transform(transform_index);
}

bool SkeletalTransformHierarchy::transform_has_parent(unsigned int transform_index) const
{
	return m_parent_indexes[transform_index] != INVALID_TRANSFORM_INDEX;
}

bool SkeletalTransformHierarchy::transform_has_parent(const char* name) const
{
	unsigned int transform_index = get_transform_index(name);
	return transform_has_parent(transform_index);
}

unsigned int SkeletalTransformHierarchy::get_transform_parent_index(unsigned int transform_index) const
{
	return m_parent_indexes[transform_index];
}

unsigned int SkeletalTransformHierarchy::get_transform_parent_index(const char* name) const
{
	unsigned int transform_index = get_transform_index(name);
	return m_parent_indexes[transform_index];
}

Matrix4 SkeletalTransformHierarchy::get_parent_transform(unsigned int transform_index) const
{
	unsigned int parent_index = m_parent_indexes[transform_index];
	if(parent_index != INVALID_TRANSFORM_INDEX){
		return m_transforms[parent_index];
	}
	else{
		return Matrix4::IDENTITY;
	}
}

Matrix4 SkeletalTransformHierarchy::get_parent_transform(const char* name) const
{
	unsigned int transform_index = get_transform_index(name);
	return get_parent_transform(transform_index);
}