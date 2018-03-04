#pragma once

#include "Engine/Math/LineSegment2.hpp"

class Capsule2{
public:
	LineSegment2	line;
	float			radius;

	Capsule2();
	Capsule2(const Capsule2& copy);
	explicit Capsule2(const LineSegment2& initialLineSegment, float initialRadius);
	explicit Capsule2(float startX, float startY, float endX, float endY, float initialRadius);

	bool				IsPointInside(const Vector2& point) const;
	Vector2				CalcClosestPoint(const Vector2& point) const;

	friend Capsule2		Interpolate(const Capsule2& start, const Capsule2& end, float fractionToEnd);
};