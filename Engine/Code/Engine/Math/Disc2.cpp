#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

Disc2::Disc2()
{
}

Disc2::Disc2(const Disc2& copy)
	:center(copy.center),
	 radius(copy.radius)
{
}

Disc2::Disc2(float initialX, float initialY, float initialRadius)
	:center(initialX, initialY),
	 radius(initialRadius)
{
}

Disc2::Disc2(const Vector2& initialCenter, float initialRadius)
	:center(initialCenter),
	 radius(initialRadius)
{
}

void Disc2::StretchToIncludePoint(const Vector2& point){
	float distanceSquared = CalcDistanceSquared(center, point);
	float radiusSquared = radius * radius;
	if(distanceSquared > radiusSquared){
		radius = sqrtf(distanceSquared);
	}
}

void Disc2::AddPadding(float paddingRadius){
	radius += paddingRadius;
}

void Disc2::Translate(const Vector2& translation){
	center += translation;
}

bool Disc2::IsPointInside(const Vector2& point) const{
	float distanceSquared = CalcDistanceSquared(center, point);
	float radiusSquared = radius * radius;
	return distanceSquared < radiusSquared;
}

bool Disc2::operator==(const Disc2& disc2ToEqual) const{
	return center == disc2ToEqual.center &&
		   radius == disc2ToEqual.radius;
}

bool Disc2::operator!=(const Disc2& disc2ToEqual) const{
	return center != disc2ToEqual.center ||
		   radius != disc2ToEqual.radius;
}

Disc2 Disc2::operator+(const Vector2& translation) const{
	return Disc2(center + translation, radius);
}

Disc2 Disc2::operator-(const Vector2& inverseTranslation) const{
	return Disc2(center - inverseTranslation, radius);
}

void Disc2::operator+=(const Vector2& translation){
	center += translation;
}

void Disc2::operator-=(const Vector2& inverseTranslation){
	center -= inverseTranslation;
}

bool DoDiscsOverlap(const Disc2& a, const Disc2& b){
	float distanceSquared = CalcDistanceSquared(a.center, b.center);

	float radiiSquared = (a.radius + b.radius) * (a.radius + b.radius);

	return distanceSquared < radiiSquared;
}

bool DoDiscsOverlap(const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius){
	return DoDiscsOverlap(Disc2(aCenter, aRadius), Disc2(bCenter, bRadius));
}

bool DoDisc2AndAABB2Overlap(const Disc2& disc, const AABB2& aabb2){
	Vector2 closestPointOnAABB2 = aabb2.CalcClosestPoint(disc.center);
	if(CalcDistanceSquared(disc.center, closestPointOnAABB2) >= (disc.radius * disc.radius))
		return false;
	else
		return true;
}

bool DoDisc2AndLineSegment2Overlap(const Disc2& disc, const LineSegment2& lineSegment){
	Vector2 closestPoint = lineSegment.CalcClosestPoint(disc.center);
	return CalcDistanceSquared(disc.center, closestPoint) < (disc.radius * disc.radius);
}

bool DoDisc2AndCapsule2Overlap(const Disc2& disc, const Capsule2& capsule){
	Vector2 closestPoint = capsule.CalcClosestPoint(disc.center);
	return CalcDistanceSquared(disc.center, closestPoint) < (disc.radius * disc.radius);
}

void KeepDiscInBounds(Disc2& disc, const AABB2& bounds)
{
    if(disc.center.x + disc.radius> bounds.maxs.x){
        disc.center.x = bounds.maxs.x - disc.radius;
    }

    if(disc.center.x - disc.radius < bounds.mins.x){
        disc.center.x = bounds.mins.x + disc.radius;
    }

    if(disc.center.y + disc.radius > bounds.maxs.y){
        disc.center.y = bounds.maxs.y - disc.radius;
    }

    if(disc.center.y - disc.radius < bounds.mins.y){
        disc.center.y = bounds.mins.y + disc.radius;
    }
}

void CorrectDiscsOverlap(Disc2& a, Disc2& b)
{
    Vector2 disp = a.center - b.center;
    float disp_len = disp.CalcLength(); 

    float sum_radii = a.radius + b.radius;
    float overlap_amount = sum_radii - disp_len;
    if(overlap_amount <= 0.0f){
        return;
    }

    float each_correction_amount = overlap_amount / 2.0f;

    Vector2 correct_dir = disp.Normalized();
    a.center += correct_dir * each_correction_amount;
    b.center -= correct_dir * each_correction_amount;
}

Disc2 Interpolate(const Disc2& start, const Disc2& end, float fractionToEnd){
	Disc2 blend;
	blend.center = Interpolate(start.center, end.center, fractionToEnd);
	blend.radius = Interpolate(start.radius, end.radius, fractionToEnd);
	return blend;
}

const Disc2 Disc2::UNIT_CIRCLE(0.f, 0.f, 1.f);