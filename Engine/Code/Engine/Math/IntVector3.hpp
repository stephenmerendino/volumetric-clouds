#pragma once

#include "Engine/Math/MathUtils.hpp"

class IntVector3{
public:
	int x;
	int y;
	int z;

	IntVector3();
	IntVector3(const IntVector3& copy);
	explicit IntVector3(int initialX, int initialY, int initialZ);

	void					GetXYZ(int& outX, int& outY, int& outZ);
	const int*				GetAsIntArray()																const;
	int*					GetAsIntArray();

	void					SetXYZ(int newX, int newY, int newZ);

	bool					operator==(const IntVector3& vectorToEqual)									const;
	bool					operator!=(const IntVector3& vectorToNotEqual)								const;
	IntVector3				operator+(const IntVector3& vectorToAdd)									const;
	IntVector3				operator-(const IntVector3& vectorToSubtract)								const;
	IntVector3				operator*(int scale)														const;
	IntVector3				operator*(float scale)														const;
	IntVector3				operator*(const IntVector3& perAxisScaleFactors)							const;
	IntVector3				operator/(int inverseScale)													const;
	IntVector3				operator-();
	void					operator*=(int scale);
	void					operator*=(float scale);
	void					operator*=(const IntVector3& perAxisScaleFactors);
	void					operator+=(const IntVector3& vectorToAdd);
	void					operator-=(const IntVector3& vectorToSubtract);
	void					operator/=(int inverseScale);

	friend IntVector3 Interpolate(const IntVector3& start, const IntVector3& end, float fractionToEnd);

	const static IntVector3 ZERO;
};

inline
IntVector3::IntVector3()
{
}

inline
IntVector3::IntVector3(const IntVector3& copy)
	:x(copy.x),
	 y(copy.y),
	 z(copy.z)
{
}

inline
IntVector3::IntVector3(int initialX, int initialY, int initialZ)
	:x(initialX),
	 y(initialY),
	 z(initialZ)
{
}

inline
void IntVector3::GetXYZ(int& outX, int& outY, int& outZ){
	outX = x;
	outY = y;
	outZ = z;
}

inline
const int* IntVector3::GetAsIntArray() const{
	return &x;
}

inline
int* IntVector3::GetAsIntArray(){
	return &x;
}

inline
void IntVector3::SetXYZ(int newX, int newY, int newZ){
	x = newX;
	y = newY;
	z = newZ;
}

inline
bool IntVector3::operator==(const IntVector3& vectorToEqual) const{
	return (x == vectorToEqual.x) && 
		   (y == vectorToEqual.y) &&
		   (z == vectorToEqual.z);
}

inline
bool IntVector3::operator!=(const IntVector3& vectorToNotEqual) const{
	return (x != vectorToNotEqual.x) || 
		   (y != vectorToNotEqual.y) ||
		   (z != vectorToNotEqual.z);
}

inline
IntVector3 IntVector3::operator+(const IntVector3& vectorToAdd) const{
	int dx = x + vectorToAdd.x;
	int dy = y + vectorToAdd.y;
	int dz = z + vectorToAdd.z;
	return IntVector3(dx, dy, dz);
}

inline
IntVector3 IntVector3::operator-(const IntVector3& vectorToSubtract) const{
	int dx = x - vectorToSubtract.x;
	int dy = y - vectorToSubtract.y;
	int dz = z - vectorToSubtract.z;
	return IntVector3(dx, dy, dz);
}

inline
void IntVector3::operator/=(int inverseScale){
	float oneOverInverseScale = 1.f / inverseScale;
	x = (int)(x * oneOverInverseScale);
	y = (int)(y * oneOverInverseScale);
	z = (int)(z * oneOverInverseScale);
}

inline
IntVector3 IntVector3::operator*(int scale) const{
	return IntVector3(x * scale, y * scale, z * scale); 
}

inline
IntVector3 IntVector3::operator*(float scale) const{
	int scaledX = (int)(x * scale);
	int scaledY = (int)(y * scale);
	int scaledZ = (int)(z * scale);
	return IntVector3(scaledX, scaledY, scaledZ);
}

inline
IntVector3 IntVector3::operator*(const IntVector3& perAxisScaleFactors) const{
	int dx = x * perAxisScaleFactors.x;
	int dy = y * perAxisScaleFactors.y;
	int dz = z * perAxisScaleFactors.z;
	return IntVector3(dx, dy, dz); 
}

inline
IntVector3 IntVector3::operator/(int inverseScale) const{
	float oneOverInverseScale = 1.f / inverseScale;
	int dx = (int)(x * oneOverInverseScale);
	int dy = (int)(y * oneOverInverseScale);
	int dz = (int)(z * oneOverInverseScale);
	return IntVector3(dx, dy, dz);
}

inline
IntVector3 IntVector3::operator-(){
	return IntVector3(-x, -y, -z);
}

inline
void IntVector3::operator*=(int scale){
	x *= scale;
	y *= scale;
	z *= scale;
}

inline
void IntVector3::operator*=(const IntVector3& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
}

inline
void IntVector3::operator*=(float scale){
	float scaledX = x * scale;
	float scaledY = y * scale;
	float scaledZ = z * scale;
	x = (int)scaledX;
	y = (int)scaledY;
	z = (int)scaledZ;
}

inline
void IntVector3::operator+=(const IntVector3& vectorToAdd){
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
}

inline
void IntVector3::operator-=(const IntVector3& vectorToSubtract){
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
}

inline
IntVector3 Interpolate(const IntVector3& start, const IntVector3& end, float fractionToEnd){
	IntVector3 interpolation;
	interpolation.x = Interpolate(start.x, end.x, fractionToEnd);
	interpolation.y = Interpolate(start.y, end.y, fractionToEnd);
	interpolation.z = Interpolate(start.z, end.z, fractionToEnd);
	return interpolation;
}