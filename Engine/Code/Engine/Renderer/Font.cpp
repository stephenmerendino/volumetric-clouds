#include "Engine/Renderer/Font.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Config/EngineConfig.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHIDevice.hpp"

#include <vector>
#include <stdlib.h>

typedef std::vector<unsigned char> FileBuffer;
typedef std::vector<unsigned char>::iterator FileIter;

Font::Font()
	:m_fontInfo(nullptr)
	,m_fontTexture(nullptr)
{
}

Font::Font(RHIDevice* device, const char* filename)
	:m_fontInfo(nullptr)
	,m_fontTexture(nullptr)
{
	LoadFromFile(device, filename);
}

Font::~Font()
{
	SAFE_DELETE_ARRAY(m_fontInfo->m_glyphs);
	SAFE_DELETE_ARRAY(m_fontInfo->m_kernings);
	SAFE_DELETE(m_fontInfo);
}

// --------------------------------------------------------------------------------------------
// Text Parsing
// --------------------------------------------------------------------------------------------

static size_t AdvanceToEndOfToken(char** out_cursor)
{
	char* startPosition = *out_cursor;

	while (**out_cursor != ' ' && 
		   **out_cursor != '=' && 
		   **out_cursor != '\"' && 
		   **out_cursor != '\r' && 
		   **out_cursor != '\n' && 
		   **out_cursor != '\0'){
		(*out_cursor)++;
	}

	return *out_cursor - startPosition;
}

static size_t AdvanceToEndOfLine(char** out_cursor)
{
	char* startPosition = *out_cursor;

	while (**out_cursor != '\r' && 
		   **out_cursor != '\n' &&
		   **out_cursor != '\0')
	{
		(*out_cursor)++;
	}

	return *out_cursor - startPosition;
}

static void AdvanceToStartOfNextToken(char** out_cursor)
{
	while ((**out_cursor == ' '  || 
		   **out_cursor == '\t'  ||
		   **out_cursor == '\r'  ||
		   **out_cursor == '\n'  ||
		   **out_cursor == '='   ||
		   **out_cursor == '\"') && **out_cursor != '\0')
	{
		(*out_cursor)++;
	}
}

static void ParseSingleToken(char** out_token, char** out_cursor)
{
	AdvanceToStartOfNextToken(out_cursor);

	char* startPos = *out_cursor;
	size_t tokenSize = AdvanceToEndOfToken(out_cursor);

	*out_token = new char[tokenSize + 1];
	memcpy(*out_token, startPos, tokenSize);
	(*out_token)[tokenSize] = '\0';
}

static void ParseSingleLine(char** out_line, char** out_cursor)
{
	AdvanceToStartOfNextToken(out_cursor);

	char* startPos = *out_cursor;
	size_t tokenSize = AdvanceToEndOfLine(out_cursor);

	*out_line = new char[tokenSize + 1];
	memcpy(*out_line, startPos, tokenSize);
	(*out_line)[tokenSize] = '\0';
}

// Check and make sure the next token equals tokenName
// This advances the cursor
static bool ValidateNextTokenAndAdvanceCursor(char** out_cursor, const char* tokenName)
{
	char* token;

	ParseSingleToken(&token, out_cursor);
	if (!AreStringsEqualCaseSensitive(token, tokenName)){
    	delete[] token;
		return false;
	}

	delete[] token;
	return true;
}

static bool ParseHeader(FontInfo** out_fontInfo, char** out_cursor)
{
	char* line;
	char* lineCursor;

	char* key;
	char* value;

	// Parse the "info" line
	{
		ValidateNextTokenAndAdvanceCursor(out_cursor, "info");

		ParseSingleLine(&line, out_cursor);
		lineCursor = line;

		while (*lineCursor != '\0'){
			ParseSingleToken(&key, &lineCursor);
			ParseSingleToken(&value, &lineCursor);

			if (AreStringsEqualCaseSensitive(key, "face")){
				(*out_fontInfo)->m_name = value;
				delete[] key;
				continue;
			}

			if (AreStringsEqualCaseSensitive(key, "size")){
				(*out_fontInfo)->m_size = (float)atof(value);
			}

			if (AreStringsEqualCaseSensitive(key, "bold")){
				(*out_fontInfo)->m_bold= (*value == '0') ? false : true;
			}

			if (AreStringsEqualCaseSensitive(key, "italic")){
				(*out_fontInfo)->m_italic= (*value == '0') ? false : true;
			}

			delete[] key;
			delete[] value;
		}

		delete[] line; // make sure to free the memory we malloc'd for the line
	}

	// Parse the "common" line
	{
		ValidateNextTokenAndAdvanceCursor(out_cursor, "common");

		ParseSingleLine(&line, out_cursor);
		lineCursor = line;

		while (*lineCursor != '\0'){
			ParseSingleToken(&key, &lineCursor);
			ParseSingleToken(&value, &lineCursor);

			if (AreStringsEqualCaseSensitive(key, "lineHeight")){
				(*out_fontInfo)->m_lineHeight = (float)atof(value) / (*out_fontInfo)->m_size;
			}

			if (AreStringsEqualCaseSensitive(key, "base")){
				(*out_fontInfo)->m_base = (float)atof(value) / (*out_fontInfo)->m_size;
			}

			if (AreStringsEqualCaseSensitive(key, "scaleW")){
				(*out_fontInfo)->m_textureWidth = atoi(value);
			}

			if (AreStringsEqualCaseSensitive(key, "scaleH")){
				(*out_fontInfo)->m_textureHeight = atoi(value);
			}

			if (AreStringsEqualCaseSensitive(key, "pages")){
				(*out_fontInfo)->m_pageCount = atoi(value);
			}

			delete[] key;
			delete[] value;
		}

		delete[] line;
	}

	// Parse the "page" line
	{
		ValidateNextTokenAndAdvanceCursor(out_cursor, "page");

		ParseSingleLine(&line, out_cursor);
		lineCursor = line;

		while (*lineCursor != '\0'){
			ParseSingleToken(&key, &lineCursor);
			ParseSingleToken(&value, &lineCursor);

			if (AreStringsEqualCaseSensitive(key, "file")){
				(*out_fontInfo)->m_textureFilename = value;
			}else{
                delete[] value;
            }

			delete[] key;
		}

		delete[] line; // make sure to free the memory we malloc'd for the line
	}

	return true;
}

static bool ParseCharacters(FontInfo** out_fontInfo, char** out_cursor)
{
	// Make sure we are where we expect to be in the file
	if (!ValidateNextTokenAndAdvanceCursor(out_cursor, "chars")){
		return false;
	}

	char* key;
	char* value;

	// Read in how many chars this font has, clean up afterwards
	{
		ParseSingleToken(&key, out_cursor);
		ParseSingleToken(&value, out_cursor);

		if (!AreStringsEqualCaseSensitive(key, "count")){
			return false;
		}

		(*out_fontInfo)->m_glyphCount = atoi(value);
		delete[] key;
		delete[] value;
	}

	// Setup the memory where we will store the FontChar's
	{
		(*out_fontInfo)->m_glyphs = new GlyphInfo[(*out_fontInfo)->m_glyphCount];
		size_t glyphInfoSize = (*out_fontInfo)->m_glyphCount * sizeof(GlyphInfo);
		memset((*out_fontInfo)->m_glyphs, 0, glyphInfoSize);
	}

	// Parse each char line in the file
	{
		char* line;
		char* lineCursor;

		for (unsigned int charIndex = 0; charIndex < (*out_fontInfo)->m_glyphCount; ++charIndex){

			ParseSingleLine(&line, out_cursor);
			lineCursor = line;

			if (!ValidateNextTokenAndAdvanceCursor(&lineCursor, "char")){
				return false;
			}

			GlyphInfo& fontChar = (*out_fontInfo)->m_glyphs[charIndex];

			while (*lineCursor != '\0'){
				ParseSingleToken(&key, &lineCursor);
				ParseSingleToken(&value, &lineCursor);

				if (AreStringsEqualCaseSensitive(key, "id")){
					fontChar.m_id = atoi(value);
				}

				if (AreStringsEqualCaseSensitive(key, "x")){
					fontChar.m_x = atoi(value);
				}

				if (AreStringsEqualCaseSensitive(key, "y")){
					fontChar.m_y = atoi(value);
				}

				if (AreStringsEqualCaseSensitive(key, "width")){
					fontChar.m_width = (float) atof(value) / (*out_fontInfo)->m_size;
				}

				if (AreStringsEqualCaseSensitive(key, "height")){
					fontChar.m_height = (float)atof(value) / (*out_fontInfo)->m_size;
				}

				if (AreStringsEqualCaseSensitive(key, "xoffset")){
					fontChar.m_xOffset = (float)atof(value) / (*out_fontInfo)->m_size;
				}

				if (AreStringsEqualCaseSensitive(key, "yoffset")){
					fontChar.m_yOffset = (float)atof(value) / (*out_fontInfo)->m_size;
				}

				if (AreStringsEqualCaseSensitive(key, "xadvance")){
					fontChar.m_xAdvance = (float)atof(value) / (*out_fontInfo)->m_size;
				}

				if (AreStringsEqualCaseSensitive(key, "page")){
					fontChar.m_pageNumber = atoi(value);
				}

				delete[] key;
				delete[] value;
			}

			// Set the AABB2 texture coordinates (for convenience use later)
			float width = (float)(*out_fontInfo)->m_textureWidth;
			float height = (float)(*out_fontInfo)->m_textureHeight;

			float widthNormlized = width / (*out_fontInfo)->m_size;
			float heightNormlized = height / (*out_fontInfo)->m_size;

			Vector2 mins((float)fontChar.m_x / width, (float)fontChar.m_y / height);
			Vector2 maxs = mins + Vector2(fontChar.m_width / widthNormlized, fontChar.m_height / heightNormlized);
			fontChar.m_texCoords = AABB2(mins, maxs);

			delete[] line; // make sure to free the memory we malloc'd for the line
		}
	}

	return true;
}

static bool ParseKernings(FontInfo** out_fontInfo, char** out_cursor)
{
	// Make sure we are where we expect to be in the file
	if (!ValidateNextTokenAndAdvanceCursor(out_cursor, "kernings")){
		return **out_cursor == '\0'; // if we hit the end of the file, thats a valid outcome since kernings are optional
	}

	char* key;
	char* value;

	// Read in how many kerning pairs this font has, clean up afterwards
	{
		ParseSingleToken(&key, out_cursor);
		ParseSingleToken(&value, out_cursor);

		if (!AreStringsEqualCaseSensitive(key, "count")){
			return false;
		}

		(*out_fontInfo)->m_kerningCount = atoi(value);
		delete[] key;
		delete[] value;
	}

	// Setup the memory where we will store the Kerning Pairs
	{
		size_t kerningsMemorySize = (*out_fontInfo)->m_kerningCount * sizeof(KerningPair);
		(*out_fontInfo)->m_kernings = new KerningPair[(*out_fontInfo)->m_kerningCount];
		memset((*out_fontInfo)->m_kernings, 0, kerningsMemorySize);
	}

	// Parse each kerning line in the file
	{
		char* line;
		char* lineCursor;

		for (unsigned int kerningIndex = 0; kerningIndex < (*out_fontInfo)->m_kerningCount; ++kerningIndex){

			ParseSingleLine(&line, out_cursor);
			lineCursor = line;

			if (!ValidateNextTokenAndAdvanceCursor(&lineCursor, "kerning")){
				return false;
			}

			KerningPair& kerningPair = (*out_fontInfo)->m_kernings[kerningIndex];

			while (*lineCursor != '\0'){
				ParseSingleToken(&key, &lineCursor);
				ParseSingleToken(&value, &lineCursor);

				if (AreStringsEqualCaseSensitive(key, "first")){
					kerningPair.m_firstId = atoi(value);
				}

				if (AreStringsEqualCaseSensitive(key, "second")){
					kerningPair.m_secondId = atoi(value);
				}

				if (AreStringsEqualCaseSensitive(key, "amount")){
					kerningPair.m_offset = (float)atof(value) / (*out_fontInfo)->m_size;
				}

				delete[] key;
				delete[] value;
			}

			delete[] line; // make sure to free the memory we malloc'd for the line
		}
	}

	return true;
}

void Font::LoadFromFile(RHIDevice* device, const char* filename)
{
	char* fileBuffer;

	std::string fullFontFilePath;
	fullFontFilePath.append(FONT_DIRECTORY);
	fullFontFilePath.append(filename);
	bool fileLoaded = LoadBinaryFileToBuffer(&fileBuffer, fullFontFilePath.c_str());

	std::string error =  Stringf("Error occurred while reading font file %s.\n", filename);
	ASSERT_OR_DIE(fileLoaded, error);

	char* cursor = fileBuffer;

	// Parse the text file into our FontInfo structure
	m_fontInfo = new FontInfo();
	ASSERT_OR_DIE(ParseHeader(&m_fontInfo, &cursor), Stringf("Error parsing header information in font file %s.\n", filename));
	ASSERT_OR_DIE(ParseCharacters(&m_fontInfo, &cursor), Stringf("Error parsing character information in font file %s.\n", filename));
	ASSERT_OR_DIE(ParseKernings(&m_fontInfo, &cursor), Stringf("Error parsing kerning information in font file %s.\n", filename));

	delete[] fileBuffer;
	fileBuffer = nullptr;

	// Need to actually load the texture
	std::string fullFontTextureFilePath;
	fullFontTextureFilePath.append(FONT_DIRECTORY);
	fullFontTextureFilePath.append(m_fontInfo->m_textureFilename);
	m_fontTexture = device->FindOrCreateRHITexture2DFromFile(fullFontTextureFilePath.c_str());

	error = Stringf("Error occurred while loading font texture for font %s.\n", filename);
	ASSERT_OR_DIE(m_fontTexture != nullptr, error);
}

const GlyphInfo* Font::GetInvalidGlyph() const
{
	return GetGlyphInfo(-1);
}

const GlyphInfo* Font::GetGlyphInfo(char c) const
{
	for (unsigned int glyphIndex = 0; glyphIndex < m_fontInfo->m_glyphCount; ++glyphIndex){
		if (m_fontInfo->m_glyphs[glyphIndex].m_id == c){
			return &m_fontInfo->m_glyphs[glyphIndex];
		}
	}

	return nullptr;
}

const KerningPair* Font::GetKerningPair(char c1, char c2) const
{
	for (unsigned int kerningIndex = 0; kerningIndex < m_fontInfo->m_kerningCount; ++kerningIndex){
		const KerningPair& kerning = m_fontInfo->m_kernings[kerningIndex];
		if (kerning.m_firstId == c1 && kerning.m_secondId == c2){
			return &kerning;
		}
	}

	return nullptr;
}

float Font::GetTextWidth(const char* text, float scale) const
{
	float cursor = 0.0f;

	const char* c = text;
	char prevCharCode = -1;

	while (*c != NULL){
		const GlyphInfo* glyph = GetGlyphInfo(*c);
		if (glyph == nullptr){
			glyph = GetInvalidGlyph();
		}

		cursor += (glyph->m_xAdvance * scale);

		prevCharCode = *c;
		++c;

		const KerningPair* kerning = GetKerningPair(prevCharCode, *c);
		if (kerning){
			cursor += (kerning->m_offset * scale);
		}
	}

	return cursor;
}

float Font::GetTextWidth(const char* text, const int numChars, float scale) const
{
	float cursor = 0.0f;

	char prevCharCode = -1;

	for(int charIndex = 0; charIndex < numChars; ++charIndex){
		char c = text[charIndex];

		const GlyphInfo* glyph = GetGlyphInfo(c);
		if (glyph == nullptr){
			glyph = GetInvalidGlyph();
		}

		cursor += (glyph->m_xAdvance * scale);

		prevCharCode = c;

		const KerningPair* kerning = GetKerningPair(prevCharCode, c);
		if (kerning){
			cursor += (kerning->m_offset * scale);
		}
	}

	return cursor;
}

float Font::GetTextWidth(const std::string& text, float scale) const
{
	float cursor = 0.0f;

	char prevCharCode = -1;

	for(unsigned int charIndex = 0; charIndex < text.size(); ++charIndex){
		char c = text[charIndex];

		const GlyphInfo* glyph = GetGlyphInfo(c);
		if (glyph == nullptr){
			glyph = GetInvalidGlyph();
		}

		cursor += (glyph->m_xAdvance * scale);

		prevCharCode = c;

		const KerningPair* kerning = GetKerningPair(prevCharCode, c);
		if (kerning){
			cursor += (kerning->m_offset * scale);
		}
	}

	return cursor;
}

float Font::GetTextWidth(const std::vector<unsigned char>& text, float scale) const
{
	float cursor = 0.0f;

	char prevCharCode = -1;

	for(unsigned int charIndex = 0; charIndex < text.size(); ++charIndex){
		unsigned char c = text[charIndex];

		const GlyphInfo* glyph = GetGlyphInfo(c);
		if (glyph == nullptr){
			glyph = GetInvalidGlyph();
		}

		cursor += (glyph->m_xAdvance * scale);

		prevCharCode = c;

		const KerningPair* kerning = GetKerningPair(prevCharCode, c);
		if (kerning){
			cursor += (kerning->m_offset * scale);
		}
	}

	return cursor;
}

float Font::GetTextWidth(const std::vector<char>& text, float scale) const
{
	float cursor = 0.0f;

	char prevCharCode = -1;

	for(unsigned int charIndex = 0; charIndex < text.size(); ++charIndex){
		char c = text[charIndex];

		const GlyphInfo* glyph = GetGlyphInfo(c);
		if (glyph == nullptr){
			glyph = GetInvalidGlyph();
		}

		cursor += (glyph->m_xAdvance * scale);

		prevCharCode = c;

		const KerningPair* kerning = GetKerningPair(prevCharCode, c);
		if (kerning){
			cursor += (kerning->m_offset * scale);
		}
	}

	return cursor;
}

float Font::GetTextHeight(const char* text, float scale) const
{
	return GetTextHeight(std::string(text), scale);
}

float Font::GetTextHeight(const std::string& text, float scale) const
{
	float largest_height = 0.0f;

	for(char c : text){
		const GlyphInfo* glyph = GetGlyphInfo(c);

		if(glyph == nullptr){
			glyph = GetInvalidGlyph();
		}

		float glyph_height = (glyph->m_yOffset + glyph->m_height) * scale;

		if(glyph_height > largest_height){
			largest_height = glyph_height;
		}
	}

	return largest_height;
}

float Font::GetLineHeight(float scale) const
{
	return m_fontInfo->m_lineHeight * scale;
}