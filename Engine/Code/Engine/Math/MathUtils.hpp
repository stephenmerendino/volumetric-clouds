#pragma once

const float		M_PI = 3.1415926535897932384626433832795f;
const float		TWO_M_PI = M_PI * 2.0f;

float			ConvertRadiansToDegrees(float radians);
float			ConvertDegreesToRadians(float degrees);

int				GetRandomIntLessThan(int maxValueNotInclusive);
int				GetRandomIntInRange(int minValueInclusive, int maxValueInclusive);
float			GetRandomFloatZeroToOne();
float			GetRandomFloatInRange(float minimumInclusive, float maximumInclusive);
bool			IsPercentChance(float probabilityZeroToOne);

float			CosDegrees(float degrees);
float			SinDegrees(float degrees);
float			TanDegrees(float degrees);
float			Atan2Degrees(float y, float x);

bool			AreMostlyEqual(float a, float b, float epsilon = 0.001f);

float			MapFloatToRange(float inputValue, float inputMin, float inputMax, float outputMin, float outputMax);
float			MapClampedFloatToRange(float inputValue, float inputMin, float inputMax, float outputMin, float outputMax);
float			MapFloatZeroToOne(float inputValue, float inputMin, float inputMax);

void			TurnToward(float& currentDegrees, float goalDegrees, float maxTurnDegrees);
float			CalcShortestAngularDistance(float startDegrees, float endDegrees);

void			KeepDegrees0To360(float& degrees);

float			Interpolate(float start, float end, float fractionToEnd);
int				Interpolate(int start, int end, float fractionToEnd);

int				FastFloorToInt(float val);

template<typename T>
T Clamp(T input, T min, T max){
	if(input < min)
		return min;

	if(input > max)
		return max;

	return input;
}

template<typename T>
T Max(T a, T b){
	return (a > b) ? a : b;
}

template<typename T>
T Min(T a, T b){
	return (a < b) ? a : b;
}

inline 
int FastFloorToInt(float val){
	int valAsInt = (int)val;

	// If val is positive or val is a whole number then just return it casted as an int
	if(val > 0.f || (val - (float)valAsInt) == 0.f)
		return valAsInt;

	// Its a negative fractional number so round down towards negative infinity
	return valAsInt - 1;
}