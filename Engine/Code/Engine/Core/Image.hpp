#pragma once

#include <string>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector2.hpp"

enum ImageLoadMode{
	IMAGE_LOAD_MODE_DEFAULT,
	IMAGE_LOAD_MODE_FORCE_ALPHA,
	NUM_IMAGE_LOAD_MODES
};

class Image{
public:
	Image(const std::string& imageFilePath, const ImageLoadMode loadMode = IMAGE_LOAD_MODE_DEFAULT);
	~Image();

	int GetWidth()								const		{ return m_width; }
	int GetHeight()								const		{ return m_height; }
	IntVector2 GetDimensions()					const		{ return IntVector2(m_width, m_height); }
	int GetNumTexels()							const		{ return m_width * m_height; }
	int GetBytesPerTexel()						const		{ return m_bytesPerTexel; }
	const unsigned char* GetImageTexelBytes()	const		{ return m_imageTexelBytes; }
    bool IsValid()                              const       { return nullptr != m_imageTexelBytes; }

	Rgba GetTexelColorAtIndex(unsigned int texelIndex);
	Rgba GetTexelColorAtPosition(unsigned int xIndex, unsigned int yIndex);

private:
	int m_width;
	int m_height;
	int m_bytesPerTexel;
	unsigned char* m_imageTexelBytes;
};