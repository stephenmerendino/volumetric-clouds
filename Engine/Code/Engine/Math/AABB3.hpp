#pragma once

#include "Engine/Math/Vector3.hpp"

class AABB3{
public:
	Vector3 mins;
	Vector3 maxs;

	AABB3();
	AABB3(const AABB3& copy);
	explicit AABB3(float initialX, float initialY, float initialZ);
	explicit AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	explicit AABB3(const Vector3& mins, const Vector3& maxs);
	explicit AABB3(const Vector3& center, float radiusX, float radiusY, float radiusZ);

	void			StretchToIncludePoint(const Vector3& point);
	void			AddPaddingToSides(float xPadRadius, float yPadRadius, float zPadRadius);
	void			Translate(const Vector3& translation);

	bool			IsPointInside(const Vector3& point)											const;
	Vector3			CalcSize()																	const;
	Vector3			CalcCenter()																const;

	Vector3			CalcClosestPoint(const Vector3& point)										const;
	Vector3			CalcBoundedPointByPercent(float xPercent, float yPercent, float zPercent)	const;
	Vector3			CalcBoundedPointByPercent(const Vector3& boundsScaleFactors)				const;

	AABB3			operator+(const Vector3& translation)										const;
	AABB3			operator-(const Vector3& inverseTranslation)								const;
	void			operator+=(const Vector3& translation);
	void			operator-=(const Vector3& inverseTranslation);

	friend bool		DoAABB3sOverlap(const AABB3& a, const AABB3& b);
	friend AABB3	Interpolate(const AABB3& start, const AABB3& end, float fractionToEnd);
	
	static const AABB3 UNIT_BOX;
	static const AABB3 NEG_ONE_TO_ONE;
};