#pragma once

#include "Engine/Math/AABB2.hpp"
#include <vector>
#include <string>

class RHIDevice;
class RHITexture2D;

struct KerningPair
{
	int m_firstId;
	int m_secondId;
	float m_offset; //normalize
};

struct GlyphInfo
{
	int m_id;

	// tex coords
	unsigned int m_x;
	unsigned int m_y;

	float m_width; // normalize
	float m_height; // normalize

	AABB2 m_texCoords;

	float m_xOffset; // normalize
	float m_yOffset; // normalize
	float m_xAdvance; //normalize

	int m_pageNumber;
};

struct FontInfo
{
	const char* m_name;
	float m_size;

	bool m_bold;
	bool m_italic;

	float m_lineHeight; // normalize
	float m_base; //normalize

	int m_textureWidth;
	int m_textureHeight;
	int m_pageCount;
	const char* m_textureFilename;

	unsigned int m_glyphCount;
	unsigned int m_kerningCount;

	GlyphInfo* m_glyphs;
	KerningPair* m_kernings;
};

// When load a font, load ALL textures (it would be an array)
// Load the num of textures

class Font
{
public:
	FontInfo*		m_fontInfo;
	RHITexture2D*	m_fontTexture;

public:
	Font();
	Font(RHIDevice* device, const char* filename);
	~Font();

	void LoadFromFile(RHIDevice* device, const char* filename);
	inline bool IsLoaded() const { return m_fontTexture != nullptr; }

	const GlyphInfo* GetInvalidGlyph() const;
	const GlyphInfo* GetGlyphInfo(char c) const;
	const KerningPair* GetKerningPair(char c1, char c2) const;

	float GetTextWidth(const char* text, float scale = 1.0f) const;
	float GetTextWidth(const char* text, const int numChars, float scale = 1.0f) const;
	float GetTextWidth(const std::string& text, float scale = 1.0f) const;
	float GetTextWidth(const std::vector<unsigned char>& text, float scale = 1.0f) const;
	float GetTextWidth(const std::vector<char>& text, float scale = 1.0f) const;

	float GetTextHeight(const char* text, float scale = 1.0f) const;
	float GetTextHeight(const std::string& text, float scale = 1.0f) const;

	float GetLineHeight(float scale = 1.0f) const;
};