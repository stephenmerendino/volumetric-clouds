#pragma once

#include "Engine/Math/Matrix4.hpp"
#include <vector>

class Pose
{
public:
	std::vector<Matrix4> m_local_transforms;
};

template<>
inline
bool BinaryStream::write(const Pose& p)
{
	bool success = true;
	success = success && write(p.m_local_transforms.size());
	for(const Matrix4& mat : p.m_local_transforms){
		success && write(mat);
	}
	return success;
}

template<>
inline
bool BinaryStream::read(Pose& p)
{
	bool success = true;

	unsigned int num_transforms = 0;
	success = success && read(num_transforms);

	p.m_local_transforms.resize(num_transforms);
	Matrix4 mat;

	for(unsigned int index = 0; index < num_transforms; ++index){
		success = success && read(mat);
		p.m_local_transforms[index] = mat;
	}

	return success;
}