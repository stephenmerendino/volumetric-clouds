#include "Engine/Math/cycle.h"
#include <math.h>

Cycle::Cycle()
{
}

Cycle::Cycle(int min, int max)
	:m_min(0)
	,m_max(0)
	,m_current(0)
	,m_range(0)
	,m_half_range(0)
{
	set_range(min, max);
}

void Cycle::set_range(int min, int max)
{
	m_min = min;

	if(max < m_min){
		m_max = m_min;
	} else{
		m_max = max;
	}

	m_range = (m_max - m_min + 1);
	m_half_range = m_range / 2;

	m_current = m_min;
}

int Cycle::get_current()
{
	return m_current;
}

void Cycle::increment()
{
	m_current++;
	if(m_current > m_max){
		m_current = m_min;
	}
}

bool Cycle::is_greater(int a, int b)
{
	int delta = abs(b - a);
	if(delta <= m_half_range){
		return (a > b);
	} else{
		return (a < b);
	}
}

uint Cycle::get_abs_delta(int a, int b)
{
	int delta = abs(b - a);
	if(delta <= m_half_range){
		return delta;
	} else{
		return m_range - delta;
	}
}