#pragma once

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/BinaryStream.hpp"
#include <math.h>

class Vector3{
public:
	float x, y, z;

	Vector3();
	Vector3(const Vector3& copy);
	Vector3(const Vector2& xy, float z);
	explicit Vector3(float initialX, float initialY, float initialZ);

	void					GetXYZ(float &outX, float &outY, float &outZ)								const;
	const float*			GetAsFloatArray()															const;
	float*					GetAsFloatArray();
	float					CalcLength()																const;
	float					CalcLengthSquared()															const;
	bool					IsMostlyEqual(const Vector3& vectorToEqual, float epsilon = 0.001f)			const;
	bool					IsMostlyEqual(float compareX, float compareY, float compareZ, float epsilon = 0.001f)	const;

	void					SetXYZ(float newX, float newY, float newZ);
	float					Normalize();
	Vector3					Normalized() const;
	float					SetLength(float newLength);
	void					ScaleUniform(float scale);
	void					ScaleNonUniform(const Vector3& perAxisScaleFactors);
	void					ScaleNonUniform(float scaleX, float scaleY, float scaleZ);
	void					InverseScaleNonUniform(const Vector3& perAxisDivisors);
	void					InverseScaleNonUniform(float inverseScaleX, float inverseScaleY, float inverseScaleZ);

	void					ClampMagnitude(float newMagnitude);
	void					ClampMagnitudeXY(float newMagnitude);

	bool					operator==(const Vector3& vectorToEqual)									const;
	bool					operator!=(const Vector3& vectorToNotEqual)									const;
	Vector3					operator+(const Vector3& vectorToAdd)										const;
	Vector3					operator-(const Vector3& vectorToSubtract)									const;
	Vector3					operator*(float scale)														const;
	Vector3					operator*(const Vector3& perAxisScaleFactors)								const;
	Vector3					operator/(float inverseScale)												const;
	Vector3					operator-()																	const;
	void					operator*=(float scale);
	void					operator*=(const Vector3& perAxisScaleFactors);
	void					operator+=(const Vector3& vectorToAdd);
	void					operator-=(const Vector3& vectorToSubtract);
	void					operator/=(float inverseScale);

	friend float			CalcDistance(const Vector3& start, const Vector3& end);
	friend float			CalcDistanceSquared(const Vector3& start, const Vector3& end);
	friend const Vector3	operator*(float scale, const Vector3& vectorToScale);
	friend float			DotProduct(const Vector3& a, const Vector3& b);
	friend Vector3			CrossProduct(const Vector3& a, const Vector3& b);
	friend bool				AreMostlyEqual(const Vector3& a, const Vector3& b, float epsilon = 0.001f);
	friend Vector3			Interpolate(const Vector3& start, const Vector3& end, float fractionToEnd);

	friend float			calc_radians_angle_between(const Vector3& a, const Vector3& b);
	friend float			calc_degrees_angle_between(const Vector3& a, const Vector3& b);
	friend Vector3			lerp(const Vector3& a, const Vector3& b, float t);
	friend Vector3			slerp(const Vector3& a, const Vector3& b, float t);

	static const Vector3	ZERO;
	static const Vector3	X_AXIS;
	static const Vector3	Y_AXIS;
	static const Vector3	Z_AXIS;
};

inline 
Vector3::Vector3()
{
}

inline 
Vector3::Vector3(const Vector3& copy)
	:x(copy.x),
	 y(copy.y),
	 z(copy.z)
{
}

inline
Vector3::Vector3(const Vector2& xy, float z)
	:x(xy.x)
	,y(xy.y)
	,z(z)
{
}

inline 
Vector3::Vector3(float initialX, float initialY, float initialZ)
	:x(initialX),
	 y(initialY),
	 z(initialZ)
{
}

inline
void Vector3::GetXYZ(float &outX, float &outY, float &outZ) const{
	outX = x;
	outY = y;
	outZ = z;
}

inline
const float* Vector3::GetAsFloatArray() const{
	return &x;
}

inline
float* Vector3::GetAsFloatArray(){
	return &x;
}

inline
float Vector3::CalcLength() const{
	return sqrtf(CalcLengthSquared());
}

inline
float Vector3::CalcLengthSquared() const{
	return (x * x) + (y * y) + (z * z);
}

inline
bool Vector3::IsMostlyEqual(const Vector3& vectorToEqual, float epsilon) const{
	float xEqual = AreMostlyEqual(x, vectorToEqual.x, epsilon);
	float yEqual = AreMostlyEqual(y, vectorToEqual.y, epsilon);
	float zEqual = AreMostlyEqual(z, vectorToEqual.z, epsilon);
	return xEqual && yEqual && zEqual;
}

inline
bool Vector3::IsMostlyEqual(float compareX, float compareY, float compareZ, float epsilon) const{
	return IsMostlyEqual(Vector3(compareX, compareY, compareZ), epsilon);
}

inline
void Vector3::SetXYZ(float newX, float newY, float newZ){
	x = newX;
	y = newY;
	z = newZ;
}

inline
Vector3	Vector3::Normalized() const{
	Vector3 copy(*this);
	copy.Normalize();
	return copy;
}

inline
float Vector3::SetLength(float newLength){
	float currentLength = CalcLength();
	float scale = newLength / currentLength;
	ScaleUniform(scale);
	return currentLength;
}

inline
void Vector3::ScaleUniform(float scale){
	x *= scale;
	y *= scale;
	z *= scale;
}

inline
void Vector3::ScaleNonUniform(const Vector3& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
}

inline
void Vector3::ScaleNonUniform(float scaleX, float scaleY, float scaleZ){
	ScaleNonUniform(Vector3(scaleX, scaleY, scaleZ));
}

inline
void Vector3::InverseScaleNonUniform(const Vector3& perAxisDivisors){
	x /= perAxisDivisors.x;
	y /= perAxisDivisors.y;
	z /= perAxisDivisors.z;
}

inline
void Vector3::InverseScaleNonUniform(float inverseScaleX, float inverseScaleY, float inverseScaleZ){
	InverseScaleNonUniform(Vector3(inverseScaleX, inverseScaleY, inverseScaleZ));
}

inline
void Vector3::ClampMagnitude(float newMagnitude){
	float currentMagnitudeSquared = CalcLengthSquared();
	float newMagnitudeSquared = newMagnitude * newMagnitude;
	if(newMagnitudeSquared < currentMagnitudeSquared){
		SetLength(newMagnitude);
	}
}

inline
void Vector3::ClampMagnitudeXY(float newMagnitude){
	float zComponent = z;
	z = 0;
	ClampMagnitude(newMagnitude);
	z = zComponent;
}

inline
bool Vector3::operator==(const Vector3& vectorToEqual) const{
	return (x == vectorToEqual.x) && 
		   (y == vectorToEqual.y) &&
		   (z == vectorToEqual.z);
}

inline
bool Vector3::operator!=(const Vector3& vectorToNotEqual) const{
	return (x != vectorToNotEqual.x) ||
		   (y != vectorToNotEqual.y) ||
		   (z != vectorToNotEqual.z);
}

inline
Vector3 Vector3::operator+(const Vector3& vectorToAdd) const{
	float dx = x + vectorToAdd.x;
	float dy = y + vectorToAdd.y;
	float dz = z + vectorToAdd.z;
	return Vector3(dx, dy, dz);
}

inline
Vector3 Vector3::operator-(const Vector3& vectorToSubtract) const{
	float dx = x - vectorToSubtract.x;
	float dy = y - vectorToSubtract.y;
	float dz = z - vectorToSubtract.z;
	return Vector3(dx, dy, dz);
}

inline
Vector3 Vector3::operator*(float scale) const{
	return Vector3(x * scale, y * scale, z * scale); 
}

inline
Vector3 Vector3::operator*(const Vector3& perAxisScaleFactors) const{
	float dx = x * perAxisScaleFactors.x;
	float dy = y * perAxisScaleFactors.y;
	float dz = z * perAxisScaleFactors.z;
	return Vector3(dx, dy, dz); 
}

inline
Vector3 Vector3::operator/(float inverseScale) const{
	float oneOverInverseScale = 1.f / inverseScale;
	float dx = x * oneOverInverseScale;
	float dy = y * oneOverInverseScale;
	float dz = z * oneOverInverseScale;
	return Vector3(dx, dy, dz);
}

inline
Vector3 Vector3::operator-() const{
	return Vector3(-x, -y, -z);
}

inline
void Vector3::operator*=(float scale){
	x *= scale;
	y *= scale;
	z *= scale;
}

inline
void Vector3::operator*=(const Vector3& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
}

inline
void Vector3::operator+=(const Vector3& vectorToAdd){
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
}

inline
void Vector3::operator-=(const Vector3& vectorToSubtract){
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
}

inline
void Vector3::operator/=(float inverseScale){
	float oneOverInverseScale = 1.f / inverseScale;
	x *= oneOverInverseScale;
	y *= oneOverInverseScale;
	z *= oneOverInverseScale;
}

inline
float CalcDistance(const Vector3& start, const Vector3& end){
	return sqrtf(CalcDistanceSquared(start, end));
}

inline
float CalcDistanceSquared(const Vector3& start, const Vector3& end){
	float dx = start.x - end.x;
	float dy = start.y - end.y;
	float dz = start.z - end.z;
	return (dx * dx) + (dy * dy) + (dz * dz);
}

inline
const Vector3 operator*(float scale, const Vector3& vectorToScale){
	return vectorToScale * scale;
}

inline
float DotProduct(const Vector3& a, const Vector3& b){
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline
Vector3 CrossProduct(const Vector3& a, const Vector3& b){
	Vector3 cross;
	cross.x = (a.y * b.z) - (a.z * b.y);
	cross.y = (a.z * b.x) - (a.x * b.z);
	cross.z = (a.x * b.y) - (a.y * b.x);
	return cross;
}

inline
bool AreMostlyEqual(const Vector3& a, const Vector3& b, float epsilon){
	return a.IsMostlyEqual(b, epsilon);
}

inline
Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionToEnd){
	float fractionOfStart = 1.f - fractionToEnd;
	return (start * fractionOfStart) + (end * fractionToEnd);
}

template<>
inline
bool BinaryStream::write(const Vector3& v)
{
	return write(v.x) && write(v.y) && write(v.z);
}

template<>
inline
bool BinaryStream::read(Vector3& v)
{
	return read(v.x) && read(v.y) && read(v.z);
}