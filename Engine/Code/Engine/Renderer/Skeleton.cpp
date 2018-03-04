#include "Engine/Renderer/Skeleton.hpp"	
#include "Engine/Renderer/SkeletonInstance.hpp"	
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Engine.hpp"

Skeleton::Skeleton()
	:m_scale(1.0f)
{
}

Skeleton::~Skeleton()
{
}

SkeletonInstance* Skeleton::create_instance()
{
	return new SkeletonInstance(this);
}

void Skeleton::clear()
{
	m_transform_hierarchy.clear();
}

void Skeleton::scale(float scale)
{
	m_scale = scale;

	for(Matrix4& transform : m_transform_hierarchy.m_transforms){
		Vector3 translation = transform.get_translation().xyz;
		translation *= scale;
		transform.set_translation(translation, 1.0f);
	}
}

void Skeleton::add_joint(const char* name, const char* parent_name, const Matrix4& transform)
{
	m_transform_hierarchy.add_transform(name, parent_name, transform);
}

unsigned int Skeleton::get_joint_count() const
{
	return m_transform_hierarchy.get_transform_count();
}

unsigned int Skeleton::get_joint_index(const char* name) const
{
	return m_transform_hierarchy.get_transform_index(name);
}

std::string Skeleton::get_joint_name(unsigned int joint_index) const
{
	return m_transform_hierarchy.get_transform_name(joint_index);
}

unsigned int Skeleton::get_joint_parent_index(unsigned int joint_index) const
{
	return m_transform_hierarchy.get_transform_parent_index(joint_index);
}

unsigned int Skeleton::get_joint_parent_index(const char* name) const
{
	return m_transform_hierarchy.get_transform_parent_index(name);
}

Matrix4 Skeleton::get_joint_transform(unsigned int joint_index) const
{
	return m_transform_hierarchy.get_transform(joint_index);
}

Matrix4 Skeleton::get_joint_transform(const char* name) const
{
	return m_transform_hierarchy.get_transform(name);
}

Matrix4 Skeleton::get_joint_parent_transform(unsigned int joint_index) const
{
	return m_transform_hierarchy.get_parent_transform(joint_index);
}

Matrix4 Skeleton::get_joint_parent_transform(const char* name) const
{
	return m_transform_hierarchy.get_parent_transform(name);
}

bool Skeleton::joint_has_parent(unsigned int joint_index) const
{
	return m_transform_hierarchy.transform_has_parent(joint_index);
}

bool Skeleton::joint_has_parent(const char* name) const
{
	return m_transform_hierarchy.transform_has_parent(name);
}

void Skeleton::draw() const
{
	g_theRenderer->draw_skeleton(this);
}

bool Skeleton::write(BinaryStream& stream)
{
	stream.m_stream_order = LITTLE_ENDIAN;

	ASSERT_OR_DIE(stream.write(m_transform_hierarchy.m_transforms.size()), "Failed to write transforms size");
	for(const Matrix4& mat : m_transform_hierarchy.m_transforms){
		ASSERT_OR_DIE(stream.write(mat), "Failed to write transform");
	}

	ASSERT_OR_DIE(stream.write(m_transform_hierarchy.m_parent_indexes.size()), "Failed to write num parent indexes");
	for(const unsigned int parent_index : m_transform_hierarchy.m_parent_indexes){
		ASSERT_OR_DIE(stream.write(parent_index), "Failed to write parent index");
	}

	return true;
}

bool Skeleton::read(BinaryStream& stream)
{
	stream.m_stream_order = LITTLE_ENDIAN;

	{
		unsigned int num_transforms;
		ASSERT_OR_DIE(stream.read(num_transforms), "Failed to read num transforms");
		m_transform_hierarchy.m_transforms.resize(num_transforms);

		for(unsigned int index = 0; index < num_transforms; ++index){
			Matrix4 transform;
			ASSERT_OR_DIE(stream.read(transform), "Failed to read transform");
			m_transform_hierarchy.m_transforms[index] = transform;
		}
	}

	{
		unsigned int num_parent_indexes;
		ASSERT_OR_DIE(stream.read(num_parent_indexes), "Failed to read num parent indexes");
		m_transform_hierarchy.m_parent_indexes.resize(num_parent_indexes);

		for(unsigned int index = 0; index < num_parent_indexes; ++index){
			unsigned int parent_index;
			ASSERT_OR_DIE(stream.read(parent_index), "Failed to read parent index");
			m_transform_hierarchy.m_parent_indexes[index] = parent_index;
		}
	}

	return true;
}