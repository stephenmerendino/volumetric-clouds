#include "Engine/Math/Quaternion.hpp"

Quaternion::Quaternion()
	:x(0.0f)
	,y(0.0f)
	,z(0.0f)
	,w(1.0f)
{
}

Quaternion::Quaternion(const Quaternion& q)
	:q_axis(q.q_axis)
	,q_angle(q.q_angle)
{
}

Quaternion::Quaternion(const Vector3& axis_of_rotation, float degrees)
{
	float half_radians = ConvertDegreesToRadians(degrees) / 2.0f;
	float sin_half_rads = sinf(half_radians);

	x = axis_of_rotation.x * sin_half_rads;
	y = axis_of_rotation.y * sin_half_rads;
	z = axis_of_rotation.z * sin_half_rads;
	w = cosf(half_radians);
}

Quaternion Quaternion::inverse() const
{
	return Quaternion(-q_axis, q_angle);
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
	float new_angle = (q_angle * other.q_angle) - DotProduct(q_axis, other.q_axis);
	Vector3 new_axis = (q_angle * other.q_axis) + (other.q_angle * q_axis) + CrossProduct(q_axis, other.q_axis);

	return Quaternion(new_axis, new_angle);
}

Vector3 Quaternion::rotate_vector(const Vector3& v) const
{
	//q * v * q^-1
	Quaternion q_v(v, 0.0f);
	return (*this * q_v * inverse()).q_axis;
}