#pragma once

#include "Engine/Math/Vector3.hpp"

class Plane3{
public:
	Vector3		m_normal;
	float		m_offset;

public:
	Plane3();
	Plane3(const Vector3& normal, float offset);

	bool AreAllPointsInFront(Vector3* points, int numPoints) const;
};