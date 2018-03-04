#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/Rgba.hpp"

class Vertex2{
public:
	Vector2		m_position;
	Rgba		m_color;
	Vector2		m_texCoords;

	Vertex2();
	Vertex2(const Vector2& initialPosition, const Rgba& initialColor, const Vector2& initialTexCoords);
	Vertex2(float initialPositionX, float initialPositionY, const Rgba& initialColor, float initialTexCoordsX, float initialTexCoordsY);
};