#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

Vector4::Vector4()
{
}

Vector4::Vector4(const Vector4& copy)
	:x(copy.x),
	 y(copy.y),
	 z(copy.z),
	 w(copy.w)
{
}

Vector4::Vector4(float initialX, float initialY, float initialZ, float initialW)
	:x(initialX),
	 y(initialY),
	 z(initialZ),
	 w(initialW)
{
}

Vector4::Vector4(const Vector3& vec3, float w)
	:x(vec3.x)
	,y(vec3.y)
	,z(vec3.z)
	,w(w)
{
}

void Vector4::GetXYZW(float &outX, float &outY, float &outZ, float &outW) const{
	outX = x;
	outY = y;
	outZ = z;
	outW = w;
}

const float* Vector4::GetAsFloatArray() const{
	return &x;
}

float* Vector4::GetAsFloatArray(){
	return &x;
}

float Vector4::CalcLength3D() const{
	return sqrtf(CalcLengthSquared3D());
}

float Vector4::CalcLengthSquared3D() const{
	return (x * x) + (y * y) + (z * z);
}

float Vector4::CalcLength4D() const{
	return sqrtf(CalcLengthSquared4D());
}

float Vector4::CalcLengthSquared4D() const{
	return (x * x) + (y * y) + (z * z) + (w * w);
}

bool Vector4::IsMostlyEqual(const Vector4& vectorToEqual, float epsilon) const{
	float xEqual = AreMostlyEqual(x, vectorToEqual.x, epsilon);
	float yEqual = AreMostlyEqual(y, vectorToEqual.y, epsilon);
	float zEqual = AreMostlyEqual(z, vectorToEqual.z, epsilon);
	float wEqual = AreMostlyEqual(w, vectorToEqual.w, epsilon);
	return xEqual && yEqual && zEqual && wEqual;
}

bool Vector4::IsMostlyEqual(float compareX, float compareY, float compareZ, float compareW, float epsilon) const{
	return IsMostlyEqual(Vector4(compareX, compareY, compareZ, compareW), epsilon);
}

void Vector4::SetXYZW(float newX, float newY, float newZ, float newW){
	x = newX;
	y = newY;
	z = newZ;
	w = newW;
}

float Vector4::Normalize3D(){
	float length = CalcLength3D();
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

Vector4	Vector4::Normalized3D() const{
	Vector4 copy(*this);
	copy.Normalize3D();
	return copy;
}

float Vector4::Normalize4D(){
	float length = CalcLength4D();
	if(length > 0.f){
		float inverseLen = 1.f / length;
		x *= inverseLen;
		y *= inverseLen;
		z *= inverseLen;
		w *= inverseLen;
		return length;
	}else{
		return 0.f;
	}
}

Vector4	Vector4::Normalized4D() const{
	Vector4 copy(*this);
	copy.Normalize4D();
	return copy;
}

float Vector4::SetLength3D(float newLength){
	float currentLength = CalcLength3D();
	float scale = newLength / currentLength;
	ScaleUniform3D(scale);
	return currentLength;
}

float Vector4::SetLength4D(float newLength){
	float currentLength = CalcLength4D();
	float scale = newLength / currentLength;
	ScaleUniform4D(scale);
	return currentLength;
}

void Vector4::ScaleUniform3D(float scale){
	x *= scale;
	y *= scale;
	z *= scale;
}

void Vector4::ScaleUniform4D(float scale){
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
}

void Vector4::ScaleNonUniform(const Vector4& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
	w *= perAxisScaleFactors.w;
}

void Vector4::ScaleNonUniform(float scaleX, float scaleY, float scaleZ, float scaleW){
	ScaleNonUniform(Vector4(scaleX, scaleY, scaleZ, scaleW));
}

void Vector4::InverseScaleNonUniform(const Vector4& perAxisDivisors){
	x /= perAxisDivisors.x;
	y /= perAxisDivisors.y;
	z /= perAxisDivisors.z;
	w /= perAxisDivisors.w;
}

void Vector4::InverseScaleNonUniform(float inverseScaleX, float inverseScaleY, float inverseScaleZ, float inverseScaleW){
	InverseScaleNonUniform(Vector4(inverseScaleX, inverseScaleY, inverseScaleZ, inverseScaleW));
}

bool Vector4::operator==(const Vector4& vectorToEqual) const{
	return (x == vectorToEqual.x) && 
		   (y == vectorToEqual.y) &&
		   (z == vectorToEqual.z) &&
		   (w == vectorToEqual.w);
}

bool Vector4::operator!=(const Vector4& vectorToNotEqual) const{
	return (x != vectorToNotEqual.x) ||
		   (y != vectorToNotEqual.y) ||
		   (z != vectorToNotEqual.z) ||
		   (w != vectorToNotEqual.w);
}

Vector4 Vector4::operator+(const Vector4& vectorToAdd) const{
	float dx = x + vectorToAdd.x;
	float dy = y + vectorToAdd.y;
	float dz = z + vectorToAdd.z;
	float dw = w + vectorToAdd.w;
	return Vector4(dx, dy, dz, dw);
}

Vector4 Vector4::operator-(const Vector4& vectorToSubtract) const{
	float dx = x - vectorToSubtract.x;
	float dy = y - vectorToSubtract.y;
	float dz = z - vectorToSubtract.z;
	float dw = w - vectorToSubtract.w;
	return Vector4(dx, dy, dz, dw);
}

Vector4 Vector4::operator*(float scale) const{
	return Vector4(x * scale, y * scale, z * scale, w * scale); 
}

Vector4 Vector4::operator*(const Vector4& perAxisScaleFactors) const{
	float dx = x * perAxisScaleFactors.x;
	float dy = y * perAxisScaleFactors.y;
	float dz = z * perAxisScaleFactors.z;
	float dw = w * perAxisScaleFactors.w;
	return Vector4(dx, dy, dz, dw); 
}

Vector4 Vector4::operator/(float inverseScale) const{
	float oneOverInverseScale = 1.f / inverseScale;
	float dx = x * oneOverInverseScale;
	float dy = y * oneOverInverseScale;
	float dz = z * oneOverInverseScale;
	float dw = w * oneOverInverseScale;
	return Vector4(dx, dy, dz, dw);
}

Vector4 Vector4::operator-(){
	return Vector4(-x, -y, -z, -w);
}

void Vector4::operator*=(float scale){
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
}

void Vector4::operator*=(const Vector4& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
	w *= perAxisScaleFactors.w;
}

void Vector4::operator+=(const Vector4& vectorToAdd){
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
	w += vectorToAdd.w;
}

void Vector4::operator-=(const Vector4& vectorToSubtract){
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
	w -= vectorToSubtract.w;
}

void Vector4::operator/=(float inverseScale){
	float oneOverInverseScale = 1.f / inverseScale;
	x *= oneOverInverseScale;
	y *= oneOverInverseScale;
	z *= oneOverInverseScale;
	w *= oneOverInverseScale;
}

float CalcDistance(const Vector4& start, const Vector4& end){
	return sqrtf(CalcDistanceSquared(start, end));
}

float CalcDistanceSquared(const Vector4& start, const Vector4& end){
	float dx = start.x - end.x;
	float dy = start.y - end.y;
	float dz = start.z - end.z;
	float dw = start.w - end.w;
	return (dx * dx) + (dy * dy) + (dz * dz) + (dw * dw);
}

const Vector4 operator*(float scale, const Vector4& vectorToScale){
	return vectorToScale * scale;
}

float DotProduct(const Vector4& a, const Vector4& b){
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

bool AreMostlyEqual(const Vector4& a, const Vector4& b, float epsilon){
	return a.IsMostlyEqual(b, epsilon);
}

Vector4 Interpolate(const Vector4& start, const Vector4& end, float fractionToEnd){
	float fractionOfStart = 1.f - fractionToEnd;
	return (start * fractionOfStart) + (end * fractionToEnd);
}

const Vector4 Vector4::ZERO(0.f, 0.f, 0.f, 0.f);