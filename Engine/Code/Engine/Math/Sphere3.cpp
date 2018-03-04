#include "Engine/Math/Sphere3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

Sphere3::Sphere3()
{
}

Sphere3::Sphere3(const Sphere3& copy)
	:center(copy.center),
	 radius(copy.radius)
{
}

Sphere3::Sphere3(float initialX, float initialY, float initialZ, float initialRadius)
	:center(initialX, initialY, initialZ),
	 radius(initialRadius)
{
}

Sphere3::Sphere3(const Vector3& initialCenter, float initialRadius)
	:center(initialCenter),
	 radius(initialRadius)
{
}

void Sphere3::StretchToIncludePoint(const Vector3& point){
	float distanceSquared = CalcDistanceSquared(center, point);
	float radiusSquared = radius * radius;
	if(distanceSquared > radiusSquared){
		radius = sqrtf(distanceSquared);
	}
}

void Sphere3::AddPadding(float paddingRadius){
	radius += paddingRadius;
}

void Sphere3::Translate(const Vector3& translation){
	center += translation;
}

bool Sphere3::IsPointInside(const Vector3& point) const{
	float distanceSquared = CalcDistanceSquared(center, point);
	return distanceSquared <= (radius * radius);
}

Sphere3 Sphere3::operator+(const Vector3& translation) const{
	return Sphere3(center + translation, radius);
}

Sphere3 Sphere3::operator-(const Vector3& inverseTranslation) const{
	return Sphere3(center - inverseTranslation, radius);
}

void Sphere3::operator+=(const Vector3& translation){
	center += translation;
}

void Sphere3::operator-=(const Vector3& inverseTranslation){
	center -= inverseTranslation;
}

bool DoSphere3sOverlap(const Sphere3& a, const Sphere3& b){
	float distanceSquared = CalcDistanceSquared(a.center, b.center);

	float radiiSquared = (a.radius + b.radius) * (a.radius + b.radius);

	return distanceSquared < radiiSquared;
}

Sphere3 Interpolate(const Sphere3& start, const Sphere3& end, float fractionToEnd){
	Sphere3 blend;
	blend.center = Interpolate(start.center, end.center, fractionToEnd);
	blend.radius = Interpolate(start.radius, end.radius, fractionToEnd);
	return blend;
}

const Sphere3 Sphere3::UNIT_SPHERE = Sphere3(Vector3::ZERO, 1.f);