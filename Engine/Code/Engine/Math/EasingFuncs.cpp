#include "Engine/Math/EasingFuncs.hpp"
#include "Engine/Math/MathUtils.hpp"

float Crossfade(EasingFunc a, EasingFunc b, float t){
	return Interpolate(a(t), b(t), t);
}

float Scale(EasingFunc a, float t){
	return a(t) * t;
}

float ReverseScale(EasingFunc a, float t){
	return a(t) * Flip(t);
}

float Flip(float t){
	return 1.f - t;
}

float Linear(float t)
{
    return t;
}

float SmoothStart2(float t){
	return (t * t);
}

float SmoothStart3(float t){
	return (t * t * t);
}

float SmoothStart4(float t){
	return (t * t * t * t);
}

float SmoothStart5(float t){
	return (t * t * t * t * t);
}

float SmoothStart6(float t){
	return (t * t * t * t * t * t);
}

float SmoothStop2(float t){
	float flip = Flip(t);
	return 1 - (flip * flip);
}

float SmoothStop3(float t){
	float flip = Flip(t);
	return 1 - (flip * flip * flip);
}

float SmoothStop4(float t){
	float flip = Flip(t);
	return 1 - (flip * flip * flip * flip);
}

float SmoothStop5(float t){
	float flip = Flip(t);
	return 1 - (flip * flip * flip *flip * flip);
}

float SmoothStop6(float t){
	float flip = Flip(t);
	return 1 - (flip * flip * flip * flip *flip * flip);
}

float SmoothStep3(float t){
	return Crossfade(SmoothStart2, SmoothStop2, t);
}

float SmoothStep5(float t){
	return Crossfade(SmoothStart4, SmoothStop4, t);
}

float SmoothStep7(float t){
	return Crossfade(SmoothStart6, SmoothStop6, t);
}