#include "Engine/Renderer/TextMeshes.hpp"
#include "Engine/Renderer/QuadMeshes.hpp"
#include "Engine/Profile/profiler.h"

void Meshes::build_text_2d(MeshBuilder& mb, 
						   const Vector2& top_left_anchor, 
						   float scale, 
						   const Rgba& tint, 
						   const char* text, 
						   const Font& font)
{
    PROFILE_SCOPE_FUNCTION();
	Vector2 cursor = top_left_anchor;

	const char* c = text;
	int prev_char = -1;

	while (*c != NULL){
		const GlyphInfo* glyph = font.GetGlyphInfo(*c);

		if (glyph == nullptr){
			glyph = font.GetInvalidGlyph();
		}

		float x_offset_scaled = (float)glyph->m_xOffset * scale;
		float y_offset_scaled = (float)glyph->m_yOffset * scale;

		float width_scaled = (float)glyph->m_width * scale;
		float height_scaled = (float)glyph->m_height * scale;

		// glyph positions in pixel space
		Vector2 top_left = cursor + Vector2(x_offset_scaled, -y_offset_scaled);
		Vector2 bottom_left = top_left + Vector2(0.0f, -height_scaled);
		Vector2 top_right = top_left + Vector2(width_scaled, 0.0f);
		Vector2 bottom_right = bottom_left + Vector2(width_scaled, 0.0f);

		AABB2 uvs = glyph->m_texCoords;

		Meshes::build_quad_2d(mb, bottom_left, top_right, uvs, tint, tint);

		cursor.x += (float)glyph->m_xAdvance * scale;

		prev_char = *c;
		++c;

		const KerningPair* kerning = font.GetKerningPair((char)prev_char, *c);
		if (kerning){
			cursor.x += (float)kerning->m_offset * scale;
		}
	}
}

void Meshes::build_text_2d(MeshBuilder& mb, 
						   const Vector2& top_left_anchor, 
						   float scale, 
						   const Rgba& tint, 
						   const std::vector<char>& text, 
						   const Font& font)
{
	Meshes::build_text_2d(mb, top_left_anchor, scale, tint, text.data(), font);
}

void Meshes::build_text_2d(MeshBuilder& mb, 
						   const Vector2& top_left_anchor, 
						   float scale, 
						   const Rgba& tint, 
						   const std::string& text, 
						   const Font& font)
{
	Meshes::build_text_2d(mb, top_left_anchor, scale, tint, text.c_str(), font);
}

void Meshes::build_text_2d_centered(MeshBuilder& mb,
									const Vector2& centered_position,
									float scale,
									const Rgba& tint,
									const std::string& text,
									const Font& font)
{
	bool is_single_char = (text.size() == 1);

	float text_width = font.GetTextWidth(text, scale);
	float text_height = font.GetTextHeight(text, scale);

	Vector2 left_anchor = centered_position + Vector2(-text_width * 0.5f, text_height * 0.5f);
		
	Vector2 cursor = left_anchor;

	const char* c = text.c_str();
	int prev_char = -1;

	while (*c != NULL){
		const GlyphInfo* glyph = font.GetGlyphInfo(*c);

		if (glyph == nullptr){
			glyph = font.GetInvalidGlyph();
		}

		float x_offset_scaled = (float)glyph->m_xOffset * scale;
		float y_offset_scaled = (float)glyph->m_yOffset * scale;

		if(is_single_char){
			y_offset_scaled *= 0.5f;
		}

		float width_scaled = (float)glyph->m_width * scale;
		float height_scaled = (float)glyph->m_height * scale;

		// glyph positions in pixel space
		Vector2 top_left = cursor + Vector2(x_offset_scaled, -y_offset_scaled);
		Vector2 bottom_left = top_left + Vector2(0.0f, -height_scaled);
		Vector2 top_right = top_left + Vector2(width_scaled, 0.0f);
		Vector2 bottom_right = bottom_left + Vector2(width_scaled, 0.0f);

		AABB2 uvs = glyph->m_texCoords;

		Meshes::build_quad_2d(mb, bottom_left, top_right, uvs, tint, tint);

		cursor.x += (float)glyph->m_xAdvance * scale;

		prev_char = *c;
		++c;

		const KerningPair* kerning = font.GetKerningPair((char)prev_char, *c);
		if (kerning){
			cursor.x += (float)kerning->m_offset * scale;
		}
	}
}

void Meshes::build_text_2d_centered_and_in_bounds(MeshBuilder& mb,
												  const AABB2& bounds,
												  const Rgba& tint,
												  const std::string& text,
												  const Font& font)
{
	Vector2 centered_pos = bounds.CalcCenter();

	float boundsWidth = bounds.CalcWidth();
	float textWidth = font.GetTextWidth(text);

	float scale = 1.0f;
	if(textWidth > boundsWidth){
		scale -= (textWidth - boundsWidth) / textWidth;
	}

	build_text_2d_centered(mb, centered_pos, scale, tint, text, font);
}

void Meshes::build_text_3d(MeshBuilder& mb,
						   const Vector3& top_left_anchor,
						   const Vector3& up,
						   const Vector3& right,
						   float scale,
						   const Rgba& tint,
						   const std::string& text,
						   const Font& font)
{
	Vector3 cursor = top_left_anchor;

	const char* c = text.c_str();
	int prev_char = -1;

	while (*c != NULL){
		const GlyphInfo* glyph = font.GetGlyphInfo(*c);

		if (glyph == nullptr){
			glyph = font.GetInvalidGlyph();
		}

		float x_offset_scaled = (float)glyph->m_xOffset * scale;
		float y_offset_scaled = (float)glyph->m_yOffset * scale;

		float width_scaled = (float)glyph->m_width * scale;
		float height_scaled = (float)glyph->m_height * scale;

		// glyph positions in pixel space
		Vector3 top_left = cursor + (right * x_offset_scaled) + (up * -y_offset_scaled);
		Vector3 bottom_left = top_left + (up * -height_scaled);
		Vector3 top_right = top_left + (right * width_scaled);
		Vector3 bottom_right = bottom_left + (right * width_scaled);

		Vector3 center = bottom_left + ((top_right - bottom_left)* 0.5f);
		float half_width = top_right.x - center.x;
		float half_height = top_right.y - center.y;

		AABB2 uvs = glyph->m_texCoords;

		Meshes::build_quad_3d(mb, center, right, up, half_width, half_height, uvs, tint, tint);

		cursor += (right * (float)glyph->m_xAdvance * scale);

		prev_char = *c;
		++c;

		const KerningPair* kerning = font.GetKerningPair((char)prev_char, *c);
		if (kerning){
			cursor += (right * (float)kerning->m_offset * scale);
		}
	}
}

void Meshes::build_text_3d_centered(MeshBuilder& mb,
									const Vector3& centered_position,
									const Vector3& up,
									const Vector3& right,
									float scale,
									const Rgba& tint,
									const std::string& text,
									const Font& font)
{
	bool is_single_char = (text.size() == 1);

	float text_width = font.GetTextWidth(text, scale);
	float text_height = font.GetTextHeight(text, scale);

	Vector3 left_anchor = centered_position + Vector3(right * -text_width * 0.5f) + Vector3(up * text_height * 0.5f);
		
	Vector3 cursor = left_anchor;

	const char* c = text.c_str();
	int prev_char = -1;

	while (*c != NULL){
		const GlyphInfo* glyph = font.GetGlyphInfo(*c);

		if(glyph == nullptr){
			glyph = font.GetInvalidGlyph();
		}

		float x_offset_scaled = (float)glyph->m_xOffset * scale;
		float y_offset_scaled = (float)glyph->m_yOffset * scale;

		if(is_single_char){
			y_offset_scaled *= 0.5f;
		}

		float width_scaled = (float)glyph->m_width * scale;
		float height_scaled = (float)glyph->m_height * scale;

		// glyph positions in pixel space
		Vector3 top_left = cursor + (right * x_offset_scaled) + (up * -y_offset_scaled);
		Vector3 bottom_left = top_left + (up * -height_scaled);
		Vector3 top_right = top_left + (right * width_scaled);
		Vector3 bottom_right = bottom_left + (right * width_scaled);

		Vector3 center = bottom_left + ((top_right - bottom_left)* 0.5f);
		float half_width = top_right.x - center.x;
		float half_height = top_right.y - center.y;

		AABB2 uvs = glyph->m_texCoords;

		Meshes::build_quad_3d(mb, center, right, up, half_width, half_height, uvs, tint, tint);

		cursor += (right * (float)glyph->m_xAdvance * scale);

		prev_char = *c;
		++c;

		const KerningPair* kerning = font.GetKerningPair((char)prev_char, *c);
		if(kerning){
			cursor += (right * (float)kerning->m_offset * scale);
		}
	}
}

void Meshes::build_text_3d_centered_and_in_bounds(MeshBuilder& mb,
												  const Vector3& centered_position,
												  const Vector3& up,
												  const Vector3& right,
												  float max_width,
												  float max_height,
												  const Rgba& tint,
												  const std::string& text,
												  const Font& font)
{
	UNUSED(max_height);
	float textWidth = font.GetTextWidth(text);

	float scale = 1.0f;
	if(textWidth > max_width){
		scale -= (textWidth - max_width) / textWidth;
	}

	build_text_3d_centered(mb, centered_position, up, right, scale, tint, text, font);
}