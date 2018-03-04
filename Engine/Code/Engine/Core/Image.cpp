#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "ThirdParty/stb/stb_image.h"

Image::Image(const std::string& imageFilePath, const ImageLoadMode loadMode)
	:m_width(0),
	 m_height(0),
	 m_bytesPerTexel(0),
	 m_imageTexelBytes(nullptr)
{
	switch (loadMode){
		case IMAGE_LOAD_MODE_FORCE_ALPHA:
		{
			m_imageTexelBytes = stbi_load(imageFilePath.c_str(), &m_width, &m_height, &m_bytesPerTexel, 4);
		} break;

		default:
		case IMAGE_LOAD_MODE_DEFAULT:
		{
			m_imageTexelBytes = stbi_load(imageFilePath.c_str(), &m_width, &m_height, &m_bytesPerTexel, 0);
		} break;
	}
}

Image::~Image(){
    free(m_imageTexelBytes);
}

Rgba Image::GetTexelColorAtIndex(unsigned int texelIndex){
	int byteOffset = texelIndex * m_bytesPerTexel;

	Rgba texelColor;
	texelColor.r = m_imageTexelBytes[byteOffset];
	texelColor.g = m_imageTexelBytes[byteOffset + 1];
	texelColor.b = m_imageTexelBytes[byteOffset + 2];

	if(m_bytesPerTexel == 4){
		texelColor.a = m_imageTexelBytes[byteOffset + 3];
	} else{
		texelColor.a = 255;
	}

	return texelColor;
}

Rgba Image::GetTexelColorAtPosition(unsigned int xIndex, unsigned int yIndex){
	unsigned int texelIndex = (yIndex * m_width) + xIndex;
	return GetTexelColorAtIndex(texelIndex);
}