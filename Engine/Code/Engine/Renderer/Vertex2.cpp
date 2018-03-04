#include "Engine/Renderer/Vertex2.hpp"

Vertex2::Vertex2()
	:m_position(Vector2::ZERO),
	 m_color(Rgba::WHITE),
	 m_texCoords(Vector2::ZERO)
{
}

Vertex2::Vertex2(const Vector2& initialPosition, const Rgba& initialColor, const Vector2& initialTexCoords)
	:m_position(initialPosition),
	 m_color(initialColor),
	 m_texCoords(initialTexCoords)
{
}

Vertex2::Vertex2(float initialPositionX, float initialPositionY, const Rgba& initialColor, float initialTexCoordsX, float initialTexCoordsY)
	:m_position(initialPositionX, initialPositionY),
	 m_color(initialColor),
	 m_texCoords(initialTexCoordsX, initialTexCoordsY)
{
}