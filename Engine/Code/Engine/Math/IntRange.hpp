#pragma once

class IntRange
{
public:
	int m_min;
	int m_max;

	IntRange();
	IntRange(int min, int max);

	int get_random_int_in_range() const;
};