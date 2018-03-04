#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"

AABB3::AABB3()
{
}

AABB3::AABB3(const AABB3& copy)
	:mins(copy.mins),
	 maxs(copy.maxs)
{
}

AABB3::AABB3(float initialX, float initialY, float initialZ)
	:mins(initialX, initialY, initialZ),
	 maxs(initialX, initialY, initialZ)
{
}

AABB3::AABB3(float minX, float minY, float minZ,float maxX, float maxY, float maxZ)
	:mins(minX, minY, minZ),
	 maxs(maxX, maxY, maxZ)
{
}

AABB3::AABB3(const Vector3& mins, const Vector3& maxs)
	:mins(mins),
	 maxs(maxs)
{
}

AABB3::AABB3(const Vector3& center, float radiusX, float radiusY, float radiusZ)
	:mins(center.x - radiusX, center.y - radiusY, center.z - radiusZ),
	 maxs(center.x + radiusX, center.y + radiusY, center.z + radiusZ)
{
}

void AABB3::StretchToIncludePoint(const Vector3& point){
	if(point.x > maxs.x){
		maxs.x = point.x;
	}

	if(point.y > maxs.y){
		maxs.y = point.y;
	}

	if(point.z > maxs.z){
		maxs.z = point.z;
	}

	if(point.x < mins.x){
		mins.x = point.x;
	}

	if(point.y < mins.y){
		mins.y = point.y;
	}

	if(point.z < mins.z){
		mins.z = point.z;
	}
}

void AABB3::AddPaddingToSides(float xPadRadius, float yPadRadius, float zPadRadius){
	mins.x -= xPadRadius;
	maxs.x += xPadRadius;

	mins.y -= yPadRadius;
	maxs.y += yPadRadius;

	mins.z -= zPadRadius;
	maxs.z += zPadRadius;
}

void AABB3::Translate(const Vector3& translation){
	mins += translation;
	maxs += translation;
}

bool AABB3::IsPointInside(const Vector3& point) const{
	if(point.x < mins.x || point.x > maxs.x){
		return false;
	}

	if(point.y < mins.y || point.y > maxs.y){
		return false;
	}

	if(point.z < mins.z || point.z > maxs.z){
		return false;
	}

	return true;
}

Vector3 AABB3::CalcSize() const{
	float xSize = maxs.x - mins.x;
	float ySize = maxs.y - mins.y;
	float zSize = maxs.z - mins.z;

	return Vector3(xSize, ySize, zSize);
}

Vector3 AABB3::CalcCenter() const{
	const Vector3 halfSize = CalcSize() * .5;

	return maxs - halfSize;
}

Vector3 AABB3::CalcClosestPoint(const Vector3& point) const{
	Vector3 closestPoint;
	closestPoint.x = Clamp(point.x, mins.x, maxs.x);
	closestPoint.y = Clamp(point.y, mins.y, maxs.y);
	closestPoint.z = Clamp(point.z, mins.z, maxs.z);
	return closestPoint;
}

Vector3 AABB3::CalcBoundedPointByPercent(const Vector3& boundsScaleFactors) const{
	Vector3 displacementScaled = (maxs - mins) * boundsScaleFactors;
	return mins + displacementScaled;
}

Vector3 AABB3::CalcBoundedPointByPercent(float xPercent, float yPercent, float zPercent) const{
	return CalcBoundedPointByPercent(Vector3(xPercent, yPercent, zPercent));
}

AABB3 AABB3::operator+(const Vector3& translation) const{
	return AABB3(mins + translation, maxs + translation);
}

AABB3 AABB3::operator-(const Vector3& inverseTranslation) const{
	return AABB3(mins - inverseTranslation, maxs - inverseTranslation);
}

void AABB3::operator+=(const Vector3& translation){
	Translate(translation);
}

void AABB3::operator-=(const Vector3& inverseTranslation){
	mins -= inverseTranslation;
	maxs -= inverseTranslation;
}

bool DoAABB3sOverlap(const AABB3& a, const AABB3& b){
	if(a.mins.x >= b.maxs.x){
		return false;
	}

	if(a.maxs.x <= b.mins.x){
		return false;
	}

	if(a.mins.y >= b.maxs.y){
		return false;
	}

	if(a.maxs.y <= b.mins.y){
		return false;
	}

	if(a.mins.z >= b.maxs.z){
		return false;
	}

	if(a.maxs.z <= b.mins.z){
		return false;
	}

	return true;
}

AABB3 Interpolate(const AABB3& start, const AABB3& end, float fractionToEnd){
	AABB3 blend;
	blend.mins = Interpolate(start.mins, end.mins, fractionToEnd);
	blend.maxs = Interpolate(start.maxs, end.maxs, fractionToEnd);
	return blend;
}

const AABB3 AABB3::UNIT_BOX(0.f, 0.f, 0.f, 1.f, 1.f, 1.f);
const AABB3 AABB3::NEG_ONE_TO_ONE(-1.f, -1.f, -1.f, 1.f, 1.f, 1.f);