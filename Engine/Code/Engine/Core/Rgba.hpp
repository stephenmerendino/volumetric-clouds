#pragma once

#include "Engine/Math/Vector4.hpp"
#include "Engine/Core/BinaryStream.hpp"

class Rgba{
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

	Rgba();
	Rgba(const std::string& color_string);
	Rgba(const Rgba& copy);
	explicit Rgba(unsigned char initialRed, 
				  unsigned char initialGreen, 
				  unsigned char initialBlue, 
				  unsigned char initialAlpha = 255);

	explicit Rgba(const Vector4& rgba);

	void SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = 255);

	void SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha = 255);
	void GetAsFloats(float &out_normalizedRed, float &out_normalizedGreen, float &out_normalizedBlue, float &out_normalizedAlpha) const;
	void GetAsFloats(float* colorArray) const;

	void ScaleRGB(float rgbScale);
	Rgba GetScaledRGB(float rgbScale) const;
	void ScaleAlpha(float alphaScale);
	Rgba GetScaledAlpha(float alphaScale) const;

	bool operator==(const Rgba& other) const;
    Rgba operator*(float scale) const;
    Rgba operator+(const Rgba& other) const;

    friend Rgba operator*(float scale, const Rgba& color);

	static const Rgba WHITE;
	static const Rgba RED;
	static const Rgba GREEN;
	static const Rgba BLUE;
	static const Rgba YELLOW;
	static const Rgba ORANGE;
	static const Rgba PINK;
	static const Rgba PURPLE;
	static const Rgba BLACK;
	static const Rgba GREY;
	static const Rgba TRANSPARENT_BLACK;
	static const Rgba TRANSPARENT_WHITE;
};

template<>
inline
bool BinaryStream::write(const Rgba& color)
{
	return  write(color.r) && 
			write(color.g) &&
			write(color.b) &&
			write(color.a);
}

template<>
inline
bool BinaryStream::read(Rgba& color)
{
	return  read(color.r) && 
			read(color.g) &&
			read(color.b) &&
			read(color.a);
}