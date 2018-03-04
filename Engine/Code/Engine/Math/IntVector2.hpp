#pragma once

#include "Engine/Math/MathUtils.hpp"
#include <math.h>

class IntVector2{
public:
	int x;
	int y;

	IntVector2();
	IntVector2(const IntVector2& copy);
	explicit IntVector2(int initialX, int initialY);
	explicit IntVector2(float x_f, float y_f);

	void					GetXY(int& outX, int& outY);
	const int*				GetAsIntArray()																const;
	int*					GetAsIntArray();

	void					SetXY(int newX, int newY);

	float					CalcLength()																const;
	int						CalcLengthSquared()															const;

	bool					operator==(const IntVector2& vectorToEqual)									const;
	bool					operator!=(const IntVector2& vectorToNotEqual)								const;
	IntVector2				operator+(const IntVector2& vectorToAdd)									const;
	IntVector2				operator-(const IntVector2& vectorToSubtract)								const;
	IntVector2				operator*(int scale)														const;
	IntVector2				operator*(float scale)														const;
	IntVector2				operator*(const IntVector2& perAxisScaleFactors)							const;
	IntVector2				operator/(int inverseScale)													const;
	IntVector2				operator-();
	void					operator*=(int scale);
	void					operator*=(float scale);
	void					operator*=(const IntVector2& perAxisScaleFactors);
	void					operator+=(const IntVector2& vectorToAdd);
	void					operator-=(const IntVector2& vectorToSubtract);
	void					operator/=(int inverseScale);

	friend IntVector2		Interpolate(const IntVector2& start, const IntVector2& end, float fractionToEnd);
	friend int				calc_manhattan_distance(const IntVector2& start, const IntVector2& end);

	const static IntVector2 ZERO;
};

inline
IntVector2::IntVector2()
{
}

inline
IntVector2::IntVector2(const IntVector2& copy)
	:x(copy.x),
	 y(copy.y)
{
}

inline
IntVector2::IntVector2(int initialX, int initialY)
	:x(initialX),
	 y(initialY)
{
}

inline
IntVector2::IntVector2(float x_f, float y_f)
	:x((int)x_f)
	,y((int)y_f)
{
}

inline
void IntVector2::GetXY(int& outX, int& outY){
	outX = x;
	outY = y;
}

inline
const int* IntVector2::GetAsIntArray() const{
	return &x;
}

inline
int* IntVector2::GetAsIntArray(){
	return &x;
}

inline
void IntVector2::SetXY(int newX, int newY){
	x = newX;
	y = newY;
}

inline
float IntVector2::CalcLength() const{
	return sqrtf((float)CalcLengthSquared());
}

inline
int	IntVector2::CalcLengthSquared() const{
	return (x * x) + (y * y);
}

inline
bool IntVector2::operator==(const IntVector2& vectorToEqual) const{
	return (x == vectorToEqual.x) && 
		   (y == vectorToEqual.y);
}

inline
bool IntVector2::operator!=(const IntVector2& vectorToNotEqual) const{
	return (x != vectorToNotEqual.x) ||
		   (y != vectorToNotEqual.y);
}

inline
IntVector2 IntVector2::operator+(const IntVector2& vectorToAdd) const{
	int dx = x + vectorToAdd.x;
	int dy = y + vectorToAdd.y;
	return IntVector2(dx, dy);
}

inline
IntVector2 IntVector2::operator-(const IntVector2& vectorToSubtract) const{
	int dx = x - vectorToSubtract.x;
	int dy = y - vectorToSubtract.y;
	return IntVector2(dx, dy);
}

inline
IntVector2 IntVector2::operator*(int scale) const{
	return IntVector2(x * scale, y * scale); 
}

inline
IntVector2 IntVector2::operator*(float scale) const{
	int scaledX = (int)(x * scale);
	int scaledY = (int)(y * scale);
	return IntVector2(scaledX, scaledY);
}

inline
IntVector2 IntVector2::operator*(const IntVector2& perAxisScaleFactors) const{
	int dx = x * perAxisScaleFactors.x;
	int dy = y * perAxisScaleFactors.y;
	return IntVector2(dx, dy); 
}

inline
IntVector2 IntVector2::operator/(int inverseScale) const{
	float oneOverInverseScale = 1.f / inverseScale;
	int dx = (int)(x * oneOverInverseScale);
	int dy = (int)(y * oneOverInverseScale);
	return IntVector2(dx, dy);
}

inline
IntVector2 IntVector2::operator-(){
	return IntVector2(-x, -y);
}

inline
void IntVector2::operator*=(int scale){
	x *= scale;
	y *= scale;
}

inline
void IntVector2::operator*=(const IntVector2& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
}

inline
void IntVector2::operator*=(float scale){
	x = (int)(x * scale);
	y = (int)(y * scale);
}

inline
void IntVector2::operator+=(const IntVector2& vectorToAdd){
	x += vectorToAdd.x;
	y += vectorToAdd.y;
}

inline
void IntVector2::operator-=(const IntVector2& vectorToSubtract){
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
}

inline
void IntVector2::operator/=(int inverseScale){
	float oneOverInverseScale = 1.f / inverseScale;
	x = (int)(x * oneOverInverseScale);
	y = (int)(y * oneOverInverseScale);
}

inline
IntVector2 Interpolate(const IntVector2& start, const IntVector2& end, float fractionToEnd){
	IntVector2 interpolation;
	interpolation.x = Interpolate(start.x, end.x, fractionToEnd);
	interpolation.y = Interpolate(start.y, end.y, fractionToEnd);
	return interpolation;
}

inline
int	calc_manhattan_distance(const IntVector2& start, const IntVector2& end)
{
	return abs(end.x - start.x) + abs(end.y - start.y);
}