#include "Engine/Math/UIntVector4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

UIntVector4::UIntVector4()
{
}

UIntVector4::UIntVector4(const UIntVector4& copy)
	:x(copy.x),
	 y(copy.y),
	 z(copy.z),
	 w(copy.w)
{
}

UIntVector4::UIntVector4(unsigned int initialX, unsigned int initialY, unsigned int initialZ, unsigned int initialW)
	:x(initialX),
	 y(initialY),
	 z(initialZ),
	 w(initialW)
{
}

bool UIntVector4::operator==(const UIntVector4& vectorToEqual) const{
	return (x == vectorToEqual.x) && 
		   (y == vectorToEqual.y) &&
		   (z == vectorToEqual.z) &&
		   (w == vectorToEqual.w);
}

bool UIntVector4::operator!=(const UIntVector4& vectorToNotEqual) const{
	return (x != vectorToNotEqual.x) ||
		   (y != vectorToNotEqual.y) ||
		   (z != vectorToNotEqual.z) ||
		   (w != vectorToNotEqual.w);
}

UIntVector4 UIntVector4::operator+(const UIntVector4& vectorToAdd) const{
	unsigned int dx = x + vectorToAdd.x;
	unsigned int dy = y + vectorToAdd.y;
	unsigned int dz = z + vectorToAdd.z;
	unsigned int dw = w + vectorToAdd.w;
	return UIntVector4(dx, dy, dz, dw);
}

UIntVector4 UIntVector4::operator-(const UIntVector4& vectorToSubtract) const{
	unsigned int dx = x - vectorToSubtract.x;
	unsigned int dy = y - vectorToSubtract.y;
	unsigned int dz = z - vectorToSubtract.z;
	unsigned int dw = w - vectorToSubtract.w;
	return UIntVector4(dx, dy, dz, dw);
}

UIntVector4 UIntVector4::operator*(const UIntVector4& perAxisScaleFactors) const{
	unsigned int dx = x * perAxisScaleFactors.x;
	unsigned int dy = y * perAxisScaleFactors.y;
	unsigned int dz = z * perAxisScaleFactors.z;
	unsigned int dw = w * perAxisScaleFactors.w;
	return UIntVector4(dx, dy, dz, dw); 
}

void UIntVector4::operator*=(const UIntVector4& perAxisScaleFactors){
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
	w *= perAxisScaleFactors.w;
}

void UIntVector4::operator+=(const UIntVector4& vectorToAdd){
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
	w += vectorToAdd.w;
}

void UIntVector4::operator-=(const UIntVector4& vectorToSubtract){
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
	w -= vectorToSubtract.w;
}

const UIntVector4 UIntVector4::ZERO(0, 0, 0, 0);