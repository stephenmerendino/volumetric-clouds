#pragma once

#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

class BitmapFont{
	friend class Renderer;

public:
	AABB2			GetGlyphTextCoordsForGlyph(int glyphUnicode) const;
	float			CalcWidth(const std::string& text, float height, float aspect);

private:
	BitmapFont(const std::string& fontName, RHITexture2D& glyphTexture);
	~BitmapFont();
	
private:
	std::string		m_fontName;
	const RHITexture2D&	m_glyphTexture;
	SpriteSheet*	m_glyphSheet;
};