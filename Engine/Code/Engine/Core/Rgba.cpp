#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"

Rgba::Rgba()
	:r(255),
	 g(255),
	 b(255),
	 a(255)
{
}

Rgba::Rgba(const std::string& color_string)
{
	if(color_string == "white") *this = Rgba::WHITE;
	if(color_string == "red") *this = Rgba::RED;
	if(color_string == "green") *this = Rgba::GREEN;
	if(color_string == "blue") *this = Rgba::BLUE;
	if(color_string == "yellow") *this = Rgba::YELLOW;
	if(color_string == "orange") *this = Rgba::ORANGE;
	if(color_string == "pink") *this = Rgba::PINK;
	if(color_string == "purple") *this = Rgba::PURPLE;
	if(color_string == "black") *this = Rgba::BLACK;
	if(color_string == "grey") *this = Rgba::GREY;
	if(color_string == "transparent_black") *this = Rgba::TRANSPARENT_BLACK;
	if(color_string == "transparent_white") *this = Rgba::TRANSPARENT_WHITE;
}

Rgba::Rgba(const Rgba& copy)
	:r(copy.r),
	 g(copy.g),
	 b(copy.b),
	 a(copy.a)
{
}

Rgba::Rgba(unsigned char initialRed,
		   unsigned char initialGreen,
		   unsigned char initialBlue,
		   unsigned char initialAlpha)
	:r(initialRed),
	 g(initialGreen),
	 b(initialBlue),
	 a(initialAlpha)
{
}

Rgba::Rgba(const Vector4& rgba)
	:r(0), g(0) ,b(0) ,a(0)
{
	SetAsFloats(rgba.x, rgba.y, rgba.z, rgba.w);
}

void Rgba::SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte){
	r = redByte;
	g = greenByte;
	b = blueByte;
	a = alphaByte;
}

void Rgba::SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha){
	float mappedRed		= MapClampedFloatToRange(normalizedRed,   0.f, 1.f, 0.f, 255.f);
	float mappedGreen	= MapClampedFloatToRange(normalizedGreen, 0.f, 1.f, 0.f, 255.f);
	float mappedBlue	= MapClampedFloatToRange(normalizedBlue,  0.f, 1.f, 0.f, 255.f);
	float mappedAlpha	= MapClampedFloatToRange(normalizedAlpha, 0.f, 1.f, 0.f, 255.f);

	r = static_cast<unsigned char>(mappedRed);
	g = static_cast<unsigned char>(mappedGreen);
	b = static_cast<unsigned char>(mappedBlue);
	a = static_cast<unsigned char>(mappedAlpha);
}

void Rgba::GetAsFloats(float &out_normalizedRed, float &out_normalizedGreen, float &out_normalizedBlue, float &out_normalizedAlpha) const{
	out_normalizedRed		= MapFloatToRange(static_cast<float>(r), 0.f, 255.f, 0.f, 1.f);
	out_normalizedGreen		= MapFloatToRange(static_cast<float>(g), 0.f, 255.f, 0.f, 1.f);
	out_normalizedBlue		= MapFloatToRange(static_cast<float>(b), 0.f, 255.f, 0.f, 1.f);
	out_normalizedAlpha		= MapFloatToRange(static_cast<float>(a), 0.f, 255.f, 0.f, 1.f);
}

void Rgba::GetAsFloats(float* colorArray) const
{
	GetAsFloats(colorArray[0], colorArray[1], colorArray[2], colorArray[3]);
}

void Rgba::ScaleRGB(float rgbScale){
	float rScaled = static_cast<float>(r) * rgbScale;
	float gScaled = static_cast<float>(g) * rgbScale;
	float bScaled = static_cast<float>(b) * rgbScale;

	r = static_cast<unsigned char>(Clamp(rScaled, 0.f, 255.f));
	g = static_cast<unsigned char>(Clamp(gScaled, 0.f, 255.f));
	b = static_cast<unsigned char>(Clamp(bScaled, 0.f, 255.f));
}

Rgba Rgba::GetScaledRGB(float rgbScale) const
{
	Rgba copy = *this;
	copy.ScaleRGB(rgbScale);
	return copy;
}

void Rgba::ScaleAlpha(float alphaScale){
	float aScaled = static_cast<float>(a) * alphaScale;

	a = static_cast<unsigned char>(Clamp(aScaled, 0.f, 255.f));
}

Rgba Rgba::GetScaledAlpha(float alphaScale) const
{
	Rgba copy = *this;
	copy.ScaleAlpha(alphaScale);
	return copy;
}

bool Rgba::operator==(const Rgba& other) const{
	return (r == other.r) &&
		   (g == other.g) &&
		   (b == other.b) &&
		   (a == other.a);
}

Rgba Rgba::operator*(float scale) const
{
    Rgba scaled;
    scaled.r = (unsigned char)((float)r * scale);
    scaled.g = (unsigned char)((float)g * scale);
    scaled.b = (unsigned char)((float)b * scale);
    scaled.a = (unsigned char)((float)a * scale);
    return scaled;
}

Rgba Rgba::operator+(const Rgba& other) const
{
    Rgba sum;
    sum.r = r + other.r;
    sum.g = g + other.g;
    sum.b = b + other.b;
    sum.a = a + other.a;
    return sum;
}

Rgba operator*(float scale, const Rgba& color)
{
    return color * scale;
}

const Rgba Rgba::WHITE		= Rgba(255, 255, 255, 255);
const Rgba Rgba::RED		= Rgba(255, 0, 0, 255);
const Rgba Rgba::GREEN		= Rgba(0, 255, 0, 255);
const Rgba Rgba::BLUE		= Rgba(0, 0, 255, 255);
const Rgba Rgba::YELLOW		= Rgba(255, 255, 0, 255);
const Rgba Rgba::ORANGE		= Rgba(255, 128, 0, 255);
const Rgba Rgba::PINK		= Rgba(255, 0, 127, 255);
const Rgba Rgba::PURPLE		= Rgba(153, 51, 255, 255);
const Rgba Rgba::BLACK		= Rgba(0, 0, 0, 255);
const Rgba Rgba::GREY		= Rgba(128, 128, 128, 255);
const Rgba Rgba::TRANSPARENT_BLACK= Rgba(0, 0, 0, 0);
const Rgba Rgba::TRANSPARENT_WHITE= Rgba(255, 255, 255, 0);