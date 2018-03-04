#pragma once

class FloatRange
{
public:
	float m_min;
	float m_max;

	FloatRange();
	FloatRange(float min, float max);

	float get_random_float_in_range() const;
};