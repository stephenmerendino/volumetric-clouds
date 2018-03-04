#pragma once

typedef float(*EasingFunc)(float);

float Crossfade(EasingFunc a, EasingFunc b, float t);
float Scale(EasingFunc a, float t);
float ReverseScale(EasingFunc a, float t);
float Flip(float t);

float Linear(float t);

float SmoothStart2(float t);
float SmoothStart3(float t);
float SmoothStart4(float t);
float SmoothStart5(float t);
float SmoothStart6(float t);

float SmoothStop2(float t);
float SmoothStop3(float t);
float SmoothStop4(float t);
float SmoothStop5(float t);
float SmoothStop6(float t);

float SmoothStep3(float t);
float SmoothStep5(float t);
float SmoothStep7(float t);