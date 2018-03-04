#include "Engine/Math/Plane3.hpp"

Plane3::Plane3()
{
}

Plane3::Plane3(const Vector3& normal, float offset)
	:m_normal(normal),
	 m_offset(offset)
{
}

bool Plane3::AreAllPointsInFront(Vector3* points, int numPoints) const{
	for(int pointIndex = 0; pointIndex < numPoints; ++pointIndex){
		if(DotProduct(points[pointIndex], m_normal) < m_offset)
			return false;
	}

	return true;
}