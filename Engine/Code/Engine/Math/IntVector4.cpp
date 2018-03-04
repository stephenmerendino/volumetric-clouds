#include "Engine/Math/IntVector4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

IntVector4::IntVector4()
{
}

IntVector4::IntVector4(const IntVector4& copy)
	:x(copy.x),
	 y(copy.y),
	 z(copy.z),
	 w(copy.w)
{
}

IntVector4::IntVector4(int initialX, int initialY, int initialZ, int initialW)
	:x(initialX),
	 y(initialY),
	 z(initialZ),
	 w(initialW)
{
}

bool IntVector4::operator==(const IntVector4& vectorToEqual) const{
	return (x == vectorToEqual.x) && 
		   (y == vectorToEqual.y) &&
		   (z == vectorToEqual.z) &&
		   (w == vectorToEqual.w);
}

bool IntVector4::operator!=(const IntVector4& vectorToNotEqual) const{
	return (x != vectorToNotEqual.x) ||
		   (y != vectorToNotEqual.y) ||
		   (z != vectorToNotEqual.z) ||
		   (w != vectorToNotEqual.w);
}

IntVector4 IntVector4::operator+(const IntVector4& vectorToAdd) const{
	int dx = x + vectorToAdd.x;
	int dy = y + vectorToAdd.y;
	int dz = z + vectorToAdd.z;
	int dw = w + vectorToAdd.w;
	return IntVector4(dx, dy, dz, dw);
}

IntVector4 IntVector4::operator-(const IntVector4& vectorToSubtract) const{
	int dx = x - vectorToSubtract.x;
	int dy = y - vectorToSubtract.y;
	int dz = z - vectorToSubtract.z;
	int dw = w - vectorToSubtract.w;
	return IntVector4(dx, dy, dz, dw);
}

IntVector4 IntVector4::operator*(const IntVector4& perAxisScaleFactors) const{
	int dx = x * perAxisScaleFactors.x;
	int dy = y * perAxisScaleFactors.y;
	int dz = z * perAxisScaleFactors.z;
	int dw = w * perAxisScaleFactors.w;
	return IntVector4(dx, dy, dz, dw); 
}

IntVector4 IntVector4::operator-(){
	return IntVector4(-x, -y, -z, -w);
}

void IntVector4::operator*=(const IntVector4& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
	w *= perAxisScaleFactors.w;
}

void IntVector4::operator+=(const IntVector4& vectorToAdd){
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
	w += vectorToAdd.w;
}

void IntVector4::operator-=(const IntVector4& vectorToSubtract){
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
	w -= vectorToSubtract.w;
}

const IntVector4 IntVector4::ZERO(0, 0, 0, 0);