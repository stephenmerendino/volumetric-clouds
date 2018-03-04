#ifndef __UTIL_H__
#define __UTIL_H__

float range_map(float value, float in_min, float in_max, float out_min, float out_max)
{
	return (((value - in_min) / (in_max - in_min)) * (out_max - out_min)) + out_min;
}

#endif