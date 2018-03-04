#pragma once

#include "Engine/Core/types.h"

class Cycle
{
	public:
		int m_min;
		int m_max;
		int m_current;
		int m_range;
		int m_half_range;

	public:
		Cycle();
		Cycle(int min, int max);

		void set_range(int min, int max);
		int get_current();
		void increment();
		bool is_greater(int a, int b);
		uint get_abs_delta(int a, int b);
};