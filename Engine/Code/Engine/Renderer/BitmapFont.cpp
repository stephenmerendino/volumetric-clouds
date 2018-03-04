#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

const IntVector2 GLYPH_SHEET_LAYOUT(16, 16);

BitmapFont::BitmapFont(const std::string& fontName, RHITexture2D& glyphTexture)
	:m_fontName(fontName),
	 m_glyphTexture(glyphTexture)
{
	m_glyphSheet = new SpriteSheet(glyphTexture, GLYPH_SHEET_LAYOUT);
}

BitmapFont::~BitmapFont(){
	delete m_glyphSheet;
}

AABB2 BitmapFont::GetGlyphTextCoordsForGlyph(int glyphUnicode) const{
	return m_glyphSheet->GetTexCoordsForSpriteIndex(glyphUnicode);
}

float BitmapFont::CalcWidth(const std::string& text, float height, float aspect){
	float charWidth = height * aspect;
	return text.size() * charWidth;
}