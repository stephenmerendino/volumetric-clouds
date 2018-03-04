#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Capsule2.hpp"

class Disc2{
public:
	Vector2 center;
	float	radius;

	Disc2();
	Disc2(const Disc2& copy);
	explicit Disc2(float initialX, float initialY, float initialRadius);
	explicit Disc2(const Vector2& initialCenter, float initialRadius);

	void			StretchToIncludePoint(const Vector2& point);
	void			AddPadding(float paddingRadius);
	void			Translate(const Vector2& translation);

	bool			IsPointInside(const Vector2& point)					const;

	bool			operator==(const Disc2& disc2ToEqual)				const;
	bool			operator!=(const Disc2& disc2ToEqual)				const;
	Disc2			operator+(const Vector2& translation)				const;
	Disc2			operator-(const Vector2& inverseTranslation)		const;
	void			operator+=(const Vector2& translation);
	void			operator-=(const Vector2& inverseTranslation);

	friend bool		DoDiscsOverlap(const Disc2& a, const Disc2& b);
	friend bool		DoDiscsOverlap(const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius);
	friend bool		DoDisc2AndAABB2Overlap(const Disc2& disc, const AABB2& aabb2);
	friend bool		DoDisc2AndLineSegment2Overlap(const Disc2& disc, const LineSegment2& lineSegment);
	friend bool		DoDisc2AndCapsule2Overlap(const Disc2& disc, const Capsule2& capsule);
	friend void		KeepDiscInBounds(Disc2& disc, const AABB2& bounds);
    friend void     CorrectDiscsOverlap(Disc2& a, Disc2& b);

	friend Disc2	Interpolate(const Disc2& start, const Disc2& end, float fractionToEnd);

	static const Disc2 UNIT_CIRCLE;
};