#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/MathUtils.hpp"

Capsule2::Capsule2()
{
}

Capsule2::Capsule2(const Capsule2& copy)
	:line(copy.line),
	 radius(copy.radius)
{
}

Capsule2::Capsule2(const LineSegment2& initialLineSegment, float initialRadius)
	:line(initialLineSegment),
	 radius(initialRadius)
{
}

Capsule2::Capsule2(float startX, float startY, float endX, float endY, float initialRadius)
	:line(startX, startY, endX, endY),
	 radius(initialRadius)
{
}

bool Capsule2::IsPointInside(const Vector2& point) const{
	Vector2 closestPointOnLine = line.CalcClosestPoint(point);
	return CalcDistanceSquared(point, closestPointOnLine) < (radius * radius);
}

Vector2 Capsule2::CalcClosestPoint(const Vector2& point) const{
	Vector2 capsuleDirection = line.CalcDirection();
	Vector2 startToPointDisplacement = point - line.start;
	Vector2 endToPointDisplacement = point - line.end;
	
	if(DotProduct(startToPointDisplacement, capsuleDirection) <= 0){
		return line.start + (startToPointDisplacement.Normalized() * radius);
	} else if(DotProduct(endToPointDisplacement, capsuleDirection) >= 0){
		return line.end + (endToPointDisplacement.Normalized() * radius);
	} else{
		Vector2 pointDisplacementAlongCapsule = DotProduct(startToPointDisplacement, capsuleDirection) * capsuleDirection;
		Vector2 pointOnLine = line.start + pointDisplacementAlongCapsule;

		Vector2 directionFromCapsulePoint = (point - pointOnLine).Normalized();

		return pointOnLine + (directionFromCapsulePoint * radius);
	}
}

Capsule2 Interpolate(const Capsule2& start, const Capsule2& end, float fractionToEnd){
	Capsule2 blend;
	blend.line = Interpolate(start.line, end.line, fractionToEnd);
	blend.radius = Interpolate(start.radius, end.radius, fractionToEnd);
	return blend;
}