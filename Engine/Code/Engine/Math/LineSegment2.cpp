#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/MathUtils.hpp"

LineSegment2::LineSegment2()
{
}

LineSegment2::LineSegment2(const LineSegment2& copy)
	:start(copy.start),
	 end(copy.end)
{
}

LineSegment2::LineSegment2(const Vector2& initialStart, const Vector2& initialEnd)
	:start(initialStart),
	 end(initialEnd)
{
}

LineSegment2::LineSegment2(float startX, float startY, float endX, float endY)
	:start(startX, startY),
	 end(endX, endY)
{
}

Vector2 LineSegment2::CalcDirection() const{
	Vector2 direction = end - start;

	direction.Normalize();

	return direction;
}

Vector2 LineSegment2::CalcDisplacement() const{
	return end - start;
}

Vector2 LineSegment2::CalcCenter() const{
	Vector2 centerDirection = CalcDisplacement() * .5f;
	return start + centerDirection;
}

Vector2 LineSegment2::CalcNormal() const{
	Vector2 directionNormalized = CalcDirection();

	directionNormalized.Rotate90Degrees();

	return directionNormalized;
}

float LineSegment2::CalcLength() const{
	Vector2 direction = CalcDisplacement();
	return direction.CalcLength();
}

float LineSegment2::CalcLengthSquared() const{
	Vector2 direction = CalcDisplacement();
	return direction.CalcLengthSquared();
}

Vector2 LineSegment2::CalcClosestPoint(const Vector2& point) const{
	Vector2 lineSegmentDirection = CalcDirection();
	Vector2 startToPointDisplacement = point - start;
	Vector2 endToPointDisplacement = point - end;
	
	if(DotProduct(startToPointDisplacement, lineSegmentDirection) <= 0){
		return start;
	} else if(DotProduct(endToPointDisplacement, lineSegmentDirection) >= 0){
		return end;
	} else{
		Vector2 pointDisplacementAlongLine = DotProduct(startToPointDisplacement, lineSegmentDirection) * lineSegmentDirection;
		return start + pointDisplacementAlongLine;
	}
}

bool LineSegment2::IsMostlyEqual(const LineSegment2& compareLineSegment) const{
	return start.IsMostlyEqual(compareLineSegment.start)
		&& end.IsMostlyEqual(compareLineSegment.end);
}

bool LineSegment2::IsMostlyEqual(const Vector2& startCompare, const Vector2& endCompare) const{
	return start.IsMostlyEqual(startCompare)
		&& end.IsMostlyEqual(endCompare);
}

bool LineSegment2::IsMostlyEqual(float compareStartX, float compareStartY, float compareEndX, float compareEndY) const{
	return start.IsMostlyEqual(compareStartX, compareStartY)
		&& end.IsMostlyEqual(compareEndX, compareEndY);
}

void LineSegment2::Translate(const Vector2& translation){
	start	+= translation;
	end		+= translation;
}

void LineSegment2::Translate(float translateX, float translateY){
	Vector2 translation(translateX, translateY);
	Translate(translation);
}

void LineSegment2::NormalizeStartAnchored(){
	Vector2 endLocalSpace = end - start;
	
	endLocalSpace.Normalize();

	end = start + endLocalSpace;
}

void LineSegment2::NormalizeEndAnchored(){
	Vector2 startLocalSpace = start - end;

	startLocalSpace.Normalize();

	start = end + startLocalSpace;
}

void LineSegment2::NormalizeCenterAnchored(){
	Vector2 centerPoint		= CalcCenter();
	Vector2 centerToEnd		= end - centerPoint;
	Vector2 centerToStart	= start - centerPoint;

	centerToEnd.SetLength(0.5f);
	centerToStart.SetLength(0.5f);

	end		= centerPoint + centerToEnd;
	start	= centerPoint + centerToStart;
}

void LineSegment2::SetLengthStartAnchored(float newLength){
	Vector2 endLocalSpace = end - start;

	endLocalSpace.SetLength(newLength);

	end = start + endLocalSpace;
}

void LineSegment2::SetLengthEndAnchored(float newLength){
	Vector2 startLocalSpace = start - end;

	startLocalSpace.SetLength(newLength);

	start = end + startLocalSpace;
}

void LineSegment2::SetLengthCenterAnchored(float newLength){
	Vector2 centerPoint		= CalcCenter();
	Vector2 centerToEnd		= end - centerPoint;
	Vector2 centerToStart	= start - centerPoint;

	centerToEnd.SetLength(newLength * .5f);
	centerToStart.SetLength(newLength * .5f);

	end		= centerPoint + centerToEnd;
	start	= centerPoint + centerToStart;
}

void LineSegment2::RotateAroundStartRadians(float radians){
	Vector2 endLocalSpace = end - start;

	endLocalSpace.RotateRadians(radians);

	end = start + endLocalSpace;
}

void LineSegment2::RotateAroundStartDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	RotateAroundStartRadians(radians);
}

void LineSegment2::RotateAroundEndRadians(float radians){
	Vector2 startLocalSpace = start - end;

	startLocalSpace.RotateRadians(radians);

	start = end + startLocalSpace;
}

void LineSegment2::RotateAroundEndDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	RotateAroundEndRadians(radians);
}

void LineSegment2::RotateAroundCenterRadians(float radians){
	Vector2 centerPoint = CalcCenter();
	Vector2 centerToEnd = end - centerPoint;
	Vector2 centerToStart = start - centerPoint;

	centerToEnd.RotateRadians(radians);
	centerToStart.RotateRadians(radians);

	start = centerPoint + centerToStart;
	end = centerPoint + centerToEnd;
}

void LineSegment2::RotateAroundCenterDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	RotateAroundCenterRadians(radians);
}

void LineSegment2::SetHeadingAroundStartRadians(float radians){
	Vector2 endLocalSpace = end - start;

	endLocalSpace.SetHeadingRadians(radians);

	end = start + endLocalSpace;
}

void LineSegment2::SetHeadingAroundStartDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	SetHeadingAroundStartRadians(radians);
}

void LineSegment2::SetHeadingAroundCenterRadians(float radians){
	Vector2 centerPoint		= CalcCenter();
	Vector2 centerToEnd		= end - centerPoint;
	Vector2 centerToStart	= start - centerPoint;

	centerToEnd.RotateRadians(radians);
	centerToStart.RotateRadians(radians);

	end		= centerPoint + centerToEnd;
	start	= centerPoint + centerToStart;
}

void LineSegment2::SetHeadingAroundCenterDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	SetHeadingAroundCenterRadians(radians);
}

void LineSegment2::SetHeadingAroundEndRadians(float radians){
	Vector2 startLocalSpace = start - end;

	startLocalSpace.SetHeadingRadians(radians);

	start = end + startLocalSpace;
}

void LineSegment2::SetHeadingAroundEndDegrees(float degrees){
	float radians = ConvertDegreesToRadians(degrees);
	SetHeadingAroundEndRadians(radians);
}

bool LineSegment2::operator==(const LineSegment2& lineSegment2ToEqual){
	return start == lineSegment2ToEqual.start &&
		   end == lineSegment2ToEqual.end;
}

bool LineSegment2::operator!=(const LineSegment2& lineSegment2ToEqual){
	return start != lineSegment2ToEqual.start ||
		   end != lineSegment2ToEqual.end;
}

LineSegment2 LineSegment2::operator*(float scale) const{
	return LineSegment2(start * scale, end * scale);
}

void LineSegment2::operator*=(float scale){
	start	*= scale;
	end		*= scale;
}

LineSegment2 LineSegment2::operator/(float scale) const{
	return LineSegment2(start / scale, end / scale);
}

void LineSegment2::operator/=(float scale){
	start	/= scale;
	end		/= scale;
}

LineSegment2 Interpolate(const LineSegment2& startLS, const LineSegment2& endLS, float fractionToEnd){
	LineSegment2 blend;
	blend.start = Interpolate(startLS.start, endLS.start, fractionToEnd);
	blend.end = Interpolate(startLS.end, endLS.end, fractionToEnd);
	return blend;
}