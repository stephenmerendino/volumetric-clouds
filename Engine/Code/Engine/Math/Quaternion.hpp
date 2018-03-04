#pragma once
#pragma warning(disable: 4201)

#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/Vector3.hpp"

class Quaternion
{
public:
	union
	{
		struct
		{
			Vector3 q_axis;
			float q_angle;
		};
		float x, y, z, w;
	};

public:
	Quaternion();
	Quaternion(const Quaternion& q);
	Quaternion(const Vector3& axis_of_rotation, float degrees);
	//Quaternion(const Matrix4& rotation_matrix);

	Quaternion inverse() const;
	Quaternion operator*(const Quaternion& other) const;
	Vector3 rotate_vector(const Vector3& v) const;

	//Matrix4 to_matrix();

	//friend Quaternion lerp(const Quaternion& a, const Quaternion& b, float t);
	//friend Quaternion slerp(const Quaternion& a, const Quaternion& b, float t);
};