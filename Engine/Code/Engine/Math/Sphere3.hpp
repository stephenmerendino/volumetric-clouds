#pragma once

#include "Engine/Math/Vector3.hpp"

class Sphere3{
public:
	Vector3 center;
	float	radius;

	Sphere3();
	Sphere3(const Sphere3& copy);
	explicit Sphere3(float initialX, float initialY, float initialZ, float initialRadius);
	explicit Sphere3(const Vector3& initialCenter, float initialRadius);

	void				StretchToIncludePoint(const Vector3& point);
	void				AddPadding(float paddingRadius);
	void				Translate(const Vector3& translation);

	bool				IsPointInside(const Vector3& point)				const;

	Sphere3				operator+(const Vector3& translation)			const;
	Sphere3				operator-(const Vector3& inverseTranslation)	const;
	void				operator+=(const Vector3& translation);
	void				operator-=(const Vector3& inverseTranslation);

	friend bool			DoSphere3sOverlap(const Sphere3& a, const Sphere3& b);
	friend Sphere3		Interpolate(const Sphere3& start, const Sphere3& end, float fractionToEnd);

	const static Sphere3 UNIT_SPHERE;
};