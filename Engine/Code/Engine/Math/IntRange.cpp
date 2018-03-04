#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"

IntRange::IntRange()
	:m_min(0)
	,m_max(0)
{
}

IntRange::IntRange(int min, int max)
	:m_min(min)
	,m_max(max)
{
}

int IntRange::get_random_int_in_range() const
{
	return GetRandomIntInRange(m_min, m_max);
}