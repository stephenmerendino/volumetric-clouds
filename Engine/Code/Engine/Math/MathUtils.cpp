#include "Engine/Math/MathUtils.hpp"
#include <math.h>
#include <stdlib.h>

float ConvertRadiansToDegrees(float radians){
	return radians * (180.f / M_PI);
}

float ConvertDegreesToRadians(float degrees){
	return degrees * (M_PI / 180.f);
}

int GetRandomIntLessThan(int maxValueNotInclusive){
	int randomInt = rand() % maxValueNotInclusive;
	return randomInt;
}

int GetRandomIntInRange(int minValueInclusive, int maxValueInclusive){
	int numPossibilities = maxValueInclusive - minValueInclusive + 1;
	return minValueInclusive + GetRandomIntLessThan(numPossibilities);
}

float GetRandomFloatZeroToOne(){
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float GetRandomFloatInRange(float minimumInclusive, float maximumInclusive){
	float numPossibilites = maximumInclusive - minimumInclusive;
	return minimumInclusive + (GetRandomFloatZeroToOne() * numPossibilites);
}

bool IsPercentChance(float probabilityZeroToOne){
	float roll = GetRandomFloatZeroToOne();
	return roll < probabilityZeroToOne;
}

float CosDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	return cosf(radians);
}

float SinDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	return sinf(radians);
}

float TanDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	return tanf(radians);
}

float Atan2Degrees(float y, float x){
	float radians = atan2f(y, x);
	return ConvertRadiansToDegrees(radians);
}

bool AreMostlyEqual(float a, float b, float epsilon){
	return fabsf(a - b) < epsilon;
}

float MapFloatToRange(float inputValue, float inputMin, float inputMax, float outputMin, float outputMax){
	float inputRange = inputMax - inputMin;
	float inputNormalized = (inputValue - inputMin) / inputRange;

	float outputRange = outputMax - outputMin;

	return (inputNormalized * outputRange) + outputMin;
}

float MapClampedFloatToRange(float inputValue, float inputMin, float inputMax, float outputMin, float outputMax){
	inputValue = Clamp(inputValue, inputMin, inputMax);
	float inputRange = inputMax - inputMin;
	float inputNormalized = (inputValue - inputMin) / inputRange;

	float outputRange = outputMax - outputMin;
	float outputValue = (inputNormalized * outputRange) + outputMin;
	return outputValue;
}

float MapFloatZeroToOne(float inputValue, float inputMin, float inputMax){
	return MapClampedFloatToRange(inputValue, inputMin, inputMax, 0.f, 1.f);
}

void TurnToward(float& currentDegrees, float goalDegrees, float maxTurnDegrees){
	float signedAngularDistance = CalcShortestAngularDistance(currentDegrees, goalDegrees);

	if(fabs(signedAngularDistance) <= maxTurnDegrees)
		currentDegrees = goalDegrees;
	else if(signedAngularDistance > 0)
		currentDegrees += maxTurnDegrees;
	else
		currentDegrees -= maxTurnDegrees;
}
float CalcShortestAngularDistance(float startDegrees, float endDegrees){
	float angularDistance = endDegrees - startDegrees;

	while(angularDistance > 180.f)
		angularDistance -= 360.f;

	while(angularDistance < -180.f)
		angularDistance += 360.f;

	return angularDistance;
}

void KeepDegrees0To360(float& degrees){
	while(degrees > 360.f)
		degrees -= 360.f;

	while(degrees < 0.f)
		degrees += 360.f;
}

float Interpolate(float start, float end, float fractionToEnd){
	float fractionOfStart = 1.f - fractionToEnd;
	return (start * fractionOfStart) + (end * fractionToEnd);
}

int	Interpolate(int start, int end, float fractionToEnd){
	float unbiasedStart = (float)start + 0.5f;
	float unbiasedEnd   = (float)end   + 0.5f;

	float fractionOfStart = 1.f - fractionToEnd;
	float unbiasedInterpolation = (unbiasedStart * fractionOfStart) + (unbiasedEnd * fractionToEnd);

	return FastFloorToInt(unbiasedInterpolation);
}