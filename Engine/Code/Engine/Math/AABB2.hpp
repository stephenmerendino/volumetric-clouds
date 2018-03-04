#pragma once

#include "Engine/Math/Vector2.hpp"

class AABB2{
public:
	Vector2 mins;
	Vector2 maxs;

	AABB2();
	AABB2(const AABB2& copy);
	explicit AABB2(float initialX, float initialY);
	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(const Vector2& mins, const Vector2& maxs);
	explicit AABB2(const Vector2& center, float radiusX, float radiusY);

	void			Scale(float scale = 1.0f);
	void			StretchToIncludePoint(const Vector2& point);
	void			AddPaddingToSides(float xPadRadius, float yPadRadius);
	void			Translate(const Vector2& translation);

	bool			IsPointInside(const Vector2& point)								const;
	Vector2			CalcSize()														const;
	Vector2			CalcCenter()													const;
	Vector2			CalcTopLeft()													const;
	Vector2			CalcBottomRight()												const;

	float			CalcWidth()														const;
	float			CalcHeight()													const;

	Vector2			CalcClosestPoint(const Vector2& point)							const;
	Vector2			CalcBoundedPointByPercent(float xPercent, float yPercent)		const;
	Vector2			CalcBoundedPointByPercent(const Vector2& boundsScaleFactors)	const;

	bool			operator==(const AABB2& aabb2ToEqual)							const;
	bool			operator!=(const AABB2& aabb2ToNotEqual)						const;
	AABB2			operator+(const Vector2& translation)							const;
	AABB2			operator-(const Vector2& inverseTranslation)					const;
	void			operator+=(const Vector2& translation);
	void			operator-=(const Vector2& inverseTranslation);
	
	friend bool		DoAABB2sOverlap(const AABB2& a, const AABB2& b);
	friend AABB2	Interpolate(const AABB2& start, const AABB2& end, float fractionToEnd);
    friend void     KeepInBounds(AABB2& inner_aabb2, const AABB2& bounds);

	static const AABB2 ZERO_TO_ONE;
	static const AABB2 NEG_ONE_TO_ONE;
};