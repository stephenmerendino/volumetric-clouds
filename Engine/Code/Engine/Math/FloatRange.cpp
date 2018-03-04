#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"

FloatRange::FloatRange()
	:m_min(0)
	,m_max(0)
{
}

FloatRange::FloatRange(float min, float max)
	:m_min(min)
	,m_max(max)
{
}

float FloatRange::get_random_float_in_range() const
{
	return GetRandomFloatInRange(m_min, m_max);
}