#pragma once

#include "Engine/Renderer/SkeletalTransformHierarchy.hpp"

class SkeletonInstance;

class Skeleton
{
public:
	float m_scale;
	SkeletalTransformHierarchy m_transform_hierarchy;

public: 
	Skeleton();
	~Skeleton();

	SkeletonInstance* create_instance();

	// Reset the skeleton - clear out all bones that make up
	// this skeleton
	void clear(); 
	void scale(float scale = 1.0f);

	// Adds a joint.  Can be parented to another 
	// joint within this skeleton.
	void add_joint(const char* name, const char* parent_name, const Matrix4& transform);

	// get number of joints/bones in this skeleton.
	unsigned int get_joint_count() const;

	// Get a joint index by name, returns
	// (uint)(-1) if it doesn't exist.
	unsigned int get_joint_index(const char* name) const;
	std::string get_joint_name(unsigned int joint_index) const;

	unsigned int get_joint_parent_index(unsigned int joint_index) const;
	unsigned int get_joint_parent_index(const char* name) const;

	// Get the global transform for a joint.
	Matrix4 get_joint_transform(unsigned int joint_index) const; 
	Matrix4 get_joint_transform(const char* name) const; 

	Matrix4 get_joint_parent_transform(unsigned int joint_index) const;
	Matrix4 get_joint_parent_transform(const char* name) const;

	bool joint_has_parent(unsigned int joint_index) const;
	bool joint_has_parent(const char* name) const;

	void draw() const;

	bool write(BinaryStream& stream);
	bool read(BinaryStream& stream);
};