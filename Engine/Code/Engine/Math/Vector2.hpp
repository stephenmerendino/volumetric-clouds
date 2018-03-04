#pragma once

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/BinaryStream.hpp"

#include <math.h>

class Vector2{
public:
	float x;
	float y;

	Vector2();
	Vector2(const Vector2& copy);
	Vector2(const IntVector2& vec2_i);
	explicit Vector2(float initialX, float initialY);

	void					GetXY(float &outX, float &outY)												const;
	const float*			GetAsFloatArray()															const;
	float*					GetAsFloatArray();
	float					CalcLength()																const;
	float					CalcLengthSquared()															const;
	float					CalcHeadingRadians()														const;
	float					CalcHeadingDegrees()														const;
	bool					IsMostlyEqual(const Vector2& vectorToEqual, float epsilon = 0.001f)			const;
	bool					IsMostlyEqual(float compareX, float compareY, float epsilon = 0.001f)		const;

	void					SetXY(float newX, float newY);

	void					Rotate90Degrees();
	void					RotateNegative90Degrees();
	void					RotateRadians(float radians);
	void					RotateDegrees(float degrees);

	float					Normalize();
	Vector2					Normalized() const;
	float					SetLength(float newLength);

	void					ScaleUniform(float scale);
	void					ScaleNonUniform(const Vector2& perAxisScaleFactors);
	void					ScaleNonUniform(float scaleX, float scaleY);
	void					InverseScaleNonUniform(const Vector2& perAxisDivisors);
	void					InverseScaleNonUniform(float inverseScaleX, float inverseScaleY);

	void					SetHeadingRadians(float headingRadians);
	void					SetHeadingDegrees(float headingDegrees);
	void					SetUnitLengthAndHeadingRadians(float headingRadians);
	void					SetUnitLengthAndHeadingDegrees(float headingDegrees);
	void					SetLengthAndHeadingRadians(float newLength, float headingRadians);
	void					SetLengthAndHeadingDegrees(float newLength, float headingDegrees);

	bool					operator==(const Vector2& vectorToEqual)									const;
	bool					operator!=(const Vector2& vectorToNotEqual)									const;
	Vector2					operator+(const Vector2& vectorToAdd)										const;
	Vector2					operator-(const Vector2& vectorToSubtract)									const;
	Vector2					operator*(float scale)														const;
	Vector2					operator*(const Vector2& perAxisScaleFactors)								const;
	Vector2					operator/(float inverseScale)												const;
	Vector2					operator-();
	void					operator*=(float scale);
	void					operator*=(const Vector2& perAxisScaleFactors);
	void					operator+=(const Vector2& vectorToAdd);
	void					operator-=(const Vector2& vectorToSubtract);
	void					operator/=(float inverseScale);

	friend float			CalcDistance(const Vector2& start, const Vector2& end);
	friend float			CalcDistanceSquared(const Vector2& start, const Vector2& end);
	friend const Vector2	operator*(float scale, const Vector2& vectorToScale);
	friend float			DotProduct(const Vector2& a, const Vector2& b);
	friend bool				AreMostlyEqual(const Vector2& a, const Vector2& b, float epsilon = 0.001f);
	friend Vector2			Interpolate(const Vector2& start, const Vector2& end, float fractionToEnd);
	friend Vector2			GetRandomDirection();
	friend void				BounceVector2(Vector2& bounceVector, const Vector2& normal);
	friend Vector2		    lerp(Vector2& a, const Vector2& b, float t);

    static Vector2          FromAngleDegrees(float degrees);
    static Vector2          FromAngleRadians(float rads);

	static const Vector2	ZERO;
	static const Vector2	ONE;

	static const Vector2	POSITIVE_X_AXIS;
	static const Vector2	NEGATIVE_X_AXIS;

	static const Vector2	POSITIVE_Y_AXIS;
	static const Vector2	NEGATIVE_Y_AXIS;
};

inline
Vector2::Vector2()
{
}

inline
Vector2::Vector2(const Vector2& copy)
	:x(copy.x),
	 y(copy.y)
{
}

inline
Vector2::Vector2(const IntVector2& vec2_i)
	:x((float)vec2_i.x)
	,y((float)vec2_i.y)
{
}

inline
Vector2::Vector2(float initialX, float initialY)
	:x(initialX),
	 y(initialY)
{
}

inline
void Vector2::GetXY(float &outX, float &outY) const{
	outX = x;
	outY = y;
}

inline
const float* Vector2::GetAsFloatArray() const{
	return &x;
}

inline
float* Vector2::GetAsFloatArray(){
	return &x;
}

inline
float Vector2::CalcLength() const{
	return sqrtf(CalcLengthSquared());
}

inline
float Vector2::CalcLengthSquared() const{
	return (x * x) + (y * y);
}

inline
float Vector2::CalcHeadingRadians() const{
	return atan2f(y, x);
}

inline
void Vector2::SetXY(float newX, float newY){
	x = newX;
	y = newY;
}

inline
void Vector2::Rotate90Degrees(){
	float oldY = y;
	y = -x;
	x = oldY;
}

inline
void Vector2::RotateNegative90Degrees(){
	float oldX = x;
	x = -y;
	y = oldX;
}

inline
void Vector2::RotateRadians(float radians){
	float currentAngle = CalcHeadingRadians();
	float length = CalcLength();
	x = length * cosf(currentAngle + radians);
	y = length * sinf(currentAngle + radians);
}

inline
Vector2	Vector2::Normalized() const{
	Vector2 copy(*this);
	copy.Normalize();
	return copy;
}

inline
float Vector2::SetLength(float newLength){
	float currentLength = CalcLength();
	float scale = newLength / currentLength;
	ScaleUniform(scale);
	return currentLength;
}

inline
void Vector2::ScaleUniform(float scale){
	x *= scale;
	y *= scale;
}

inline
void Vector2::ScaleNonUniform(const Vector2& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
}

inline
void Vector2::ScaleNonUniform(float scaleX, float scaleY){
	x *= scaleX;
	y *= scaleY;
}

inline
void Vector2::InverseScaleNonUniform(const Vector2& perAxisDivisors){
	x /= perAxisDivisors.x;
	y /= perAxisDivisors.y;
}

inline
void Vector2::InverseScaleNonUniform(float inverseScaleX, float inverseScaleY){
	x /= inverseScaleX;
	y /= inverseScaleY;
}

inline
void Vector2::SetHeadingRadians(float headingRadians){
	float currentLength = CalcLength();
	x = currentLength * cosf(headingRadians);
	y = currentLength * sinf(headingRadians);
}

inline
void Vector2::SetUnitLengthAndHeadingRadians(float headingRadians){
	x = cosf(headingRadians);
	y = sinf(headingRadians);
}

inline
void Vector2::SetLengthAndHeadingRadians(float newLength, float headingRadians){
	x = newLength * cosf(headingRadians);
	y = newLength * sinf(headingRadians);
}

inline
bool Vector2::operator==(const Vector2& vectorToEqual) const{
	return (x == vectorToEqual.x) && 
		   (y == vectorToEqual.y);
}

inline
bool Vector2::operator!=(const Vector2& vectorToNotEqual) const{
	return (x != vectorToNotEqual.x) ||
		   (y != vectorToNotEqual.y);
}

inline
Vector2 Vector2::operator+(const Vector2& vectorToAdd) const{
	float dx = x + vectorToAdd.x;
	float dy = y + vectorToAdd.y;
	return Vector2(dx, dy);
}

inline
Vector2 Vector2::operator-(const Vector2& vectorToSubtract) const{
	float dx = x - vectorToSubtract.x;
	float dy = y - vectorToSubtract.y;
	return Vector2(dx, dy);
}

inline
Vector2 Vector2::operator*(float scale) const{
	return Vector2(x * scale, y * scale); 
}

inline
Vector2 Vector2::operator*(const Vector2& perAxisScaleFactors) const{
	float dx = x * perAxisScaleFactors.x;
	float dy = y * perAxisScaleFactors.y;
	return Vector2(dx, dy); 
}

inline
Vector2 Vector2::operator/(float inverseScale) const{
	float scaleFactor = 1.f / inverseScale;
	float dx = x * scaleFactor;
	float dy = y * scaleFactor;
	return Vector2(dx, dy);
}

inline
Vector2 Vector2::operator-(){
	return Vector2(-x, -y);
}

inline
void Vector2::operator*=(float scale){
	x *= scale;
	y *= scale;
}

inline
void Vector2::operator*=(const Vector2& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
}

inline
void Vector2::operator+=(const Vector2& vectorToAdd){
	x += vectorToAdd.x;
	y += vectorToAdd.y;
}

inline
void Vector2::operator-=(const Vector2& vectorToSubtract){
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
}

inline
void Vector2::operator/=(float inverseScale){
	float oneOverInverseScale = 1.f / inverseScale;
	x *= oneOverInverseScale;
	y *= oneOverInverseScale;
}

inline
float CalcDistance(const Vector2& start, const Vector2& end){
	return sqrtf(CalcDistanceSquared(start, end));
}

inline
float CalcDistanceSquared(const Vector2& start, const Vector2& end){
	float dx = start.x - end.x;
	float dy = start.y - end.y;
	return (dx * dx) + (dy * dy);
}

inline
const Vector2 operator*(float scale, const Vector2& vectorToScale){
	return vectorToScale * scale;
}

inline
float DotProduct(const Vector2& a, const Vector2& b){
	return (a.x * b.x) + (a.y * b.y);
}

inline
bool AreMostlyEqual(const Vector2& a, const Vector2& b, float epsilon){
	return a.IsMostlyEqual(b, epsilon);
}

inline
Vector2	Interpolate(const Vector2& start, const Vector2& end, float fractionToEnd){
	float fractionOfStart = 1.f - fractionToEnd;
	return (start * fractionOfStart) + (end * fractionToEnd);
}

inline
Vector2 GetRandomDirection(){
	Vector2 unitLengthVector(0.f, 1.f);

	float randomDirectionDegrees = GetRandomFloatInRange(0.f, 360.f);
	unitLengthVector.SetHeadingDegrees(randomDirectionDegrees);

	return unitLengthVector;
}

inline
void BounceVector2(Vector2& bounceVector, const Vector2& normal){
	Vector2 alongNormal = DotProduct(bounceVector, normal) * normal;
	Vector2 tangentNormal = bounceVector - alongNormal;

	bounceVector = -alongNormal + tangentNormal;
}

inline
Vector2 lerp(Vector2& a, const Vector2& b, float t)
{
    return ((1.0f - t) * a + (b * t));
}

inline
float Vector2::CalcHeadingDegrees() const{
	return Atan2Degrees(y, x);
}

inline
bool Vector2::IsMostlyEqual(const Vector2& vectorToEqual, float epsilon) const{
	float xEqual = AreMostlyEqual(x, vectorToEqual.x, epsilon);
	float yEqual = AreMostlyEqual(y, vectorToEqual.y, epsilon);
	return xEqual && yEqual;
}

inline
bool Vector2::IsMostlyEqual(float compareX, float compareY, float epsilon) const{
	return IsMostlyEqual(Vector2(compareX, compareY), epsilon);
}

inline
void Vector2::RotateDegrees(float degrees){
	RotateRadians(ConvertDegreesToRadians(degrees));
}

inline
void Vector2::SetHeadingDegrees(float headingDegrees){
	SetHeadingRadians(ConvertDegreesToRadians(headingDegrees));
}

inline
void Vector2::SetLengthAndHeadingDegrees(float newLength, float headingDegrees){
	SetLengthAndHeadingRadians(newLength, ConvertDegreesToRadians(headingDegrees));
}

inline
void Vector2::SetUnitLengthAndHeadingDegrees(float headingDegrees){
	SetUnitLengthAndHeadingRadians(ConvertDegreesToRadians(headingDegrees));
}

template<>
inline
bool BinaryStream::write(const Vector2& v)
{
	return write(v.x) && write(v.y);
}

template<>
inline
bool BinaryStream::read(Vector2& v)
{
	return read(v.x) && read(v.y);
}