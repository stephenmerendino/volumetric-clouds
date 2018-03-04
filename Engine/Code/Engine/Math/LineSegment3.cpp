#include "Engine/Math/LineSegment3.hpp"

LineSegment3::LineSegment3()
{
}

LineSegment3::LineSegment3(const LineSegment3& copy)
	:start(copy.start),
	 end(copy.end)
{
}

LineSegment3::LineSegment3(const Vector3& initialStart, const Vector3& initialEnd)
	:start(initialStart),
	 end(initialEnd)
{
}

LineSegment3::LineSegment3(float startX, float startY, float startZ, float endX, float endY, float endZ)
	:start(startX, startY, startZ),
	 end(endX, endY, endZ)
{
}

Vector3 LineSegment3::CalcDirection() const{
	Vector3 direction = end - start;

	direction.Normalize();

	return direction;
}

Vector3 LineSegment3::CalcDisplacement() const{
	return end - start;
}

Vector3 LineSegment3::CalcCenter() const{
	Vector3 centerDirection = CalcDisplacement() * .5f;
	return start + centerDirection;
}

float LineSegment3::CalcLength() const{
	Vector3 direction = CalcDisplacement();
	return direction.CalcLength();
}

float LineSegment3::CalcLengthSquared() const{
	Vector3 direction = CalcDisplacement();
	return direction.CalcLengthSquared();
}

bool LineSegment3::IsMostlyEqual(const LineSegment3& compareLineSegment) const{
	return start.IsMostlyEqual(compareLineSegment.start)
		&& end.IsMostlyEqual(compareLineSegment.end);
}

bool LineSegment3::IsMostlyEqual(const Vector3& startCompare, const Vector3& endCompare) const{
	return start.IsMostlyEqual(startCompare)
		&& end.IsMostlyEqual(endCompare);
}

bool LineSegment3::IsMostlyEqual(float compareStartX, float compareStartY, float compareStartZ, 
								 float compareEndX,	  float compareEndY,   float compareEndZ) const{
	return start.IsMostlyEqual(compareStartX, compareStartY, compareStartZ)
		&& end.IsMostlyEqual(compareEndX, compareEndY, compareEndZ);
}

void LineSegment3::Translate(const Vector3& translation){
	start	+= translation;
	end		+= translation;
}

void LineSegment3::Translate(float translateX, float translateY, float translateZ){
	Vector3 translation(translateX, translateY, translateZ);
	Translate(translation);
}

LineSegment3 Interpolate(const LineSegment3& startLS, const LineSegment3& endLS, float fractionToEnd){
	LineSegment3 blend;
	blend.start = Interpolate(startLS.start, endLS.start, fractionToEnd);
	blend.end = Interpolate(startLS.end, endLS.end, fractionToEnd);
	return blend;
}