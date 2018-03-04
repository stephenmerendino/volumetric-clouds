#pragma once

#include <string>
#include "Engine/Math/IntVector2.hpp"

class Texture
{
	friend class Renderer;

private:
	Texture();
	~Texture();

private:
	std::string			m_imageFilePath;
	unsigned int		m_textureID;
	IntVector2			m_texelDimensions;
};