#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

AABB2::AABB2()
{
}

AABB2::AABB2(const AABB2& copy)
	:mins(copy.mins),
	 maxs(copy.maxs)
{
}

AABB2::AABB2(float initialX, float initialY)
	:mins(initialX, initialY),
	 maxs(initialX, initialY)
{
}

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	:mins(minX, minY),
	 maxs(maxX, maxY)
{
}

AABB2::AABB2(const Vector2& mins, const Vector2& maxs)
	:mins(mins),
	 maxs(maxs)
{
}

AABB2::AABB2(const Vector2& center, float radiusX, float radiusY)
	:mins(center.x - radiusX, center.y - radiusY),
	 maxs(center.x + radiusX, center.y + radiusY)
{
}

void AABB2::Scale(float scale)
{
	Vector2 center = CalcCenter();
	Vector2 disp_to_mins = (mins - center);
	Vector2 disp_to_maxs = (maxs - center);

	mins = center + (disp_to_mins * scale);
	maxs = center + (disp_to_maxs * scale);
}

void AABB2::StretchToIncludePoint(const Vector2& point){
	if(point.x > maxs.x){
		maxs.x = point.x;
	}

	if(point.y > maxs.y){
		maxs.y = point.y;
	}

	if(point.x < mins.x){
		mins.x = point.x;
	}

	if(point.y < mins.y){
		mins.y = point.y;
	}
}

void AABB2::AddPaddingToSides(float xPadRadius, float yPadRadius){
	mins.x -= xPadRadius;
	maxs.x += xPadRadius;

	mins.y -= yPadRadius;
	maxs.y += yPadRadius;
}

void AABB2::Translate(const Vector2& translation){
	mins += translation;
	maxs += translation;
}

bool AABB2::IsPointInside(const Vector2& point) const{
	if(point.x <= mins.x || point.x >= maxs.x){
		return false;
	}

	if(point.y <= mins.y || point.y >= maxs.y){
		return false;
	}

	return true;
}

Vector2 AABB2::CalcSize() const{
	float xSize = maxs.x - mins.x;
	float ySize = maxs.y - mins.y;

	return Vector2(xSize, ySize);
}

Vector2 AABB2::CalcCenter() const{
	const Vector2 halfSize = CalcSize() * .5;

	return maxs - halfSize;
}

Vector2 AABB2::CalcTopLeft() const{
	return Vector2(mins.x, maxs.y);
}

Vector2 AABB2::CalcBottomRight() const{
	return Vector2(maxs.x, mins.y);
}

float AABB2::CalcWidth() const
{
	return maxs.x - mins.x;
}

float AABB2::CalcHeight() const
{
	return maxs.y - mins.y;
}

Vector2 AABB2::CalcClosestPoint(const Vector2& point) const{
	Vector2 closestPoint;
	closestPoint.x = Clamp(point.x, mins.x, maxs.x);
	closestPoint.y = Clamp(point.y, mins.y, maxs.y);
	return closestPoint;
}


Vector2 AABB2::CalcBoundedPointByPercent(const Vector2& boundsScaleFactors) const{
	Vector2 displacementScaled = (maxs - mins) * boundsScaleFactors;
	return mins + displacementScaled;
}

Vector2 AABB2::CalcBoundedPointByPercent(float xPercent, float yPercent) const{
	return CalcBoundedPointByPercent(Vector2(xPercent, yPercent));
}

bool AABB2::operator==(const AABB2& aabb2ToEqual) const{
	return mins == aabb2ToEqual.mins &&
		   maxs == aabb2ToEqual.maxs;
}

bool AABB2::operator!=(const AABB2& aabb2ToNotEqual) const{
	return mins != aabb2ToNotEqual.mins ||
		   maxs != aabb2ToNotEqual.maxs;
}

AABB2 AABB2::operator+(const Vector2& translation) const{
	return AABB2(mins + translation, maxs + translation);
}

AABB2 AABB2::operator-(const Vector2& inverseTranslation) const{
	return AABB2(mins - inverseTranslation, maxs - inverseTranslation);
}

void AABB2::operator+=(const Vector2& translation){
	Translate(translation);
}

void AABB2::operator-=(const Vector2& inverseTranslation){
	mins -= inverseTranslation;
	maxs -= inverseTranslation;
}

bool DoAABB2sOverlap(const AABB2& a, const AABB2& b){
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

	return true;
}

AABB2 Interpolate(const AABB2& start, const AABB2& end, float fractionToEnd){
	AABB2 blend;
	blend.mins = Interpolate(start.mins, end.mins, fractionToEnd);
	blend.maxs = Interpolate(start.maxs, end.maxs, fractionToEnd);
	return blend;
}

void KeepInBounds(AABB2& inner_aabb2, const AABB2& bounds)
{
    Vector2 correction(0.0, 0.0);

    if(inner_aabb2.mins.x < bounds.mins.x){
        correction.x = (bounds.mins.x - inner_aabb2.mins.x);
    }else if(inner_aabb2.maxs.x > bounds.maxs.x){
        correction.x = (bounds.maxs.x - inner_aabb2.maxs.x);
    }

    if(inner_aabb2.mins.y < bounds.mins.y){
        correction.y = (bounds.mins.y - inner_aabb2.mins.y);
    }else if(inner_aabb2.maxs.y > bounds.maxs.y){
        correction.y = (bounds.maxs.y - inner_aabb2.maxs.y);
    }

    inner_aabb2.Translate(correction);
}

const AABB2 AABB2::ZERO_TO_ONE(0.f, 0.f, 1.f, 1.f);
const AABB2 AABB2::NEG_ONE_TO_ONE(-1.f, -1.f, 1.f, 1.f);