#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"

float Vector2::Normalize(){
	float length = CalcLength();
	if(length > 0.f){
		float inverseLen = 1.f / length;
		x *= inverseLen;
		y *= inverseLen;
		return length;
	}else{
		return 0.f;
	}
}

Vector2 Vector2::FromAngleDegrees(float degrees)
{
    return Vector2(CosDegrees(degrees), SinDegrees(degrees));
}

Vector2 Vector2::FromAngleRadians(float rads)
{
    return Vector2(cosf(rads), sinf(rads));
}

const Vector2 Vector2::ZERO(0.f, 0.f);
const Vector2 Vector2::ONE(1.f, 1.f);

const Vector2 Vector2::POSITIVE_X_AXIS(1.f, 0.f);
const Vector2 Vector2::NEGATIVE_X_AXIS(-1.f, 0.f);

const Vector2 Vector2::POSITIVE_Y_AXIS(0.f, 1.f);
const Vector2 Vector2::NEGATIVE_Y_AXIS(0.f, -1.f);