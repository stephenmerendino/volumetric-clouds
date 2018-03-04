#pragma once

#include "Engine/Math/Vector2.hpp"

class LineSegment2{
public:
	Vector2 start;
	Vector2 end;

	LineSegment2();
	LineSegment2(const LineSegment2& copy);
	explicit LineSegment2(const Vector2& intialStart, const Vector2& initialEnd);
	explicit LineSegment2(float startX, float startY, float endX, float endY);

	Vector2				CalcDisplacement()											const;
	Vector2				CalcDirection()												const;

	Vector2				CalcCenter()												const;
	Vector2				CalcNormal()												const;
	float				CalcLength()												const;
	float				CalcLengthSquared()											const;
	Vector2				CalcClosestPoint(const Vector2& point)						const;

	bool				IsMostlyEqual(const LineSegment2& compareLineSegment)		const;

	bool				IsMostlyEqual(const Vector2& startCompare, 
		                              const Vector2& endCompare)					const;

	bool				IsMostlyEqual(float compareStartX, float compareStartY, 
									  float compareEndX, float compareEndY)			const;

	void				Translate(const Vector2& translation);
	void				Translate(float translateX, float translateY);

	void				NormalizeStartAnchored();
	void				NormalizeEndAnchored();
	void				NormalizeCenterAnchored();

	void				SetLengthStartAnchored(float newLength);
	void				SetLengthEndAnchored(float newLength);
	void				SetLengthCenterAnchored(float newLength);

	void				RotateAroundStartDegrees(float degrees);
	void				RotateAroundStartRadians(float radians);

	void				RotateAroundEndRadians(float radians);
	void				RotateAroundEndDegrees(float degrees);

	void				RotateAroundCenterRadians(float radians);
	void				RotateAroundCenterDegrees(float degrees);

	void				SetHeadingAroundStartRadians(float radians);
	void				SetHeadingAroundStartDegrees(float degrees);

	void				SetHeadingAroundCenterRadians(float radians);
	void				SetHeadingAroundCenterDegrees(float degrees);

	void				SetHeadingAroundEndRadians(float radians);
	void				SetHeadingAroundEndDegrees(float degrees);

	bool				operator==(const LineSegment2& lineSegment2ToEqual);
	bool				operator!=(const LineSegment2& lineSegment2ToEqual);
	
	LineSegment2		operator*(float scale)										const;
	void				operator*=(float scale);

	LineSegment2		operator/(float scale)										const;
	void				operator/=(float scale);

	friend LineSegment2	Interpolate(const LineSegment2& startLS, const LineSegment2& endLS, float fractionToEnd);
};