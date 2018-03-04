#include "Engine/Math/Vector3.hpp"

float Vector3::Normalize(){
	float length = CalcLength();
	if(length > 0.f){
		float inverseLen = 1.f / length;
		x *= inverseLen;
		y *= inverseLen;
		z *= inverseLen;
		return length;
	}else{
		return 0.f;
	}
}

float calc_radians_angle_between(const Vector3& a, const Vector3& b)
{
	float dot = DotProduct(a.Normalized(), b.Normalized());
	dot = Max(dot, -1.0f);
	dot = Min(dot, 1.0f);
	return (float)acos(dot);
}

float calc_degrees_angle_between(const Vector3& a, const Vector3& b)
{
	return ConvertRadiansToDegrees(calc_radians_angle_between(a, b));
}

Vector3 lerp(const Vector3& a, const Vector3& b, float t)
{
	return ((1.0f - t) * a) + (t * b);
}

Vector3 slerp(const Vector3& a, const Vector3& b, float t)
{
	float radians_angle_between = calc_radians_angle_between(a, b);
	float sin_angle = sinf(radians_angle_between);

	if(fabs(sin_angle) < 0.001f){
		return a;
	}

	Vector3 a_factor = (sinf((1.0f - t) * radians_angle_between) / sin_angle) * a;
	Vector3 b_factor = (sinf(t * radians_angle_between) / sin_angle) * b;

	return a_factor + b_factor;
}

const Vector3 Vector3::ZERO(0.f, 0.f, 0.f);
const Vector3 Vector3::X_AXIS(1.f, 0.f, 0.f);
const Vector3 Vector3::Y_AXIS(0.f, 1.f, 0.f);
const Vector3 Vector3::Z_AXIS(0.f, 0.f, 1.f);