#pragma once

#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Engine.hpp"

namespace Meshes
{
	void build_text_2d(MeshBuilder& mb, 
					   const Vector2& top_left_anchor, 
					   float scale, 
					   const Rgba& tint, 
					   const char* text, 
					   const Font& font);

	void build_text_2d(MeshBuilder& mb, 
					   const Vector2& top_left_anchor, 
					   float scale, 
					   const Rgba& tint, 
					   const std::vector<char>& text, 
					   const Font& font);

	void build_text_2d(MeshBuilder& mb, 
					   const Vector2& top_left_anchor, 
					   float scale, 
					   const Rgba& tint, 
					   const std::string& text, 
					   const Font& font);

	void build_text_2d_centered(MeshBuilder& mb, 
								const Vector2& centered_position, 
								float scale, 
								const Rgba& tint, 
								const std::string& text, 
								const Font& font);

	void build_text_2d_centered_and_in_bounds(MeshBuilder& mb, 
											  const AABB2& bounds, 
											  const Rgba& tint, 
											  const std::string& text, 
											  const Font& font);

	void build_text_3d(MeshBuilder& mb,
					  const Vector3& top_left_anchor,
					  const Vector3& up,
					  const Vector3& right,
					  float scale,
					  const Rgba& tint,
					  const std::string& text,
					  const Font& font);

	void build_text_3d_centered(MeshBuilder& mb, 
								const Vector3& centered_position, 
								const Vector3& up,
								const Vector3& right,
								float scale, 
								const Rgba& tint, 
								const std::string& text, 
								const Font& font);

	void build_text_3d_centered_and_in_bounds(MeshBuilder& mb, 
											  const Vector3& centered_position, 
											  const Vector3& up,
											  const Vector3& right,
											  float max_width,
											  float max_height,
											  const Rgba& tint, 
											  const std::string& text, 
											  const Font& font);
}