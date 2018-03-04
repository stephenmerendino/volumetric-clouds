#pragma once

#include "Engine/Math/Vector3.hpp"

class LineSegment3{
public:
	Vector3 start;
	Vector3 end;

	LineSegment3();
	LineSegment3(const LineSegment3& copy);
	explicit LineSegment3(const Vector3& intialStart, const Vector3& initialEnd);
	explicit LineSegment3(float startX, float startY, float startZ, float endX, float endY, float endZ);

	Vector3				CalcDisplacement()											const;
	Vector3				CalcDirection()												const;

	Vector3				CalcCenter()												const;
	float				CalcLength()												const;
	float				CalcLengthSquared()											const;

	bool				IsMostlyEqual(const LineSegment3& compareLineSegment)		const;

	bool				IsMostlyEqual(const Vector3& startCompare, 
		                              const Vector3& endCompare)					const;

	bool				IsMostlyEqual(float compareStartX, float compareStartY, float compareStartZ,
									  float compareEndX, float compareEndY, float compareEndZ) const;

	void				Translate(const Vector3& translation);
	void				Translate(float translateX, float translateY, float translateZ);

	friend LineSegment3 Interpolate(const LineSegment3& startLS, const LineSegment3& endLS, float fractionToEnd);
};
