#include "Engine/Renderer/SpriteSheet.hpp"

SpriteSheet::SpriteSheet(RHITexture2D& texture, const IntVector2& spriteSheetLayout)
	:m_spriteSheetTexture(texture),
	 m_spriteLayout(spriteSheetLayout),
	 m_tileDimensions(1.0f / spriteSheetLayout.x, 1.0f / spriteSheetLayout.y)
{
}

SpriteSheet::SpriteSheet(RHITexture2D& texture, int spriteSheetLayoutWidth, int spriteSheetLayoutHeight)
	:SpriteSheet(texture, IntVector2(spriteSheetLayoutWidth, spriteSheetLayoutHeight))
{
}

AABB2 SpriteSheet::GetTexCoordsForSpriteCoords(int spriteX, int spriteY) const{
	Vector2 tileMins;
	tileMins.x = spriteX * m_tileDimensions.x;
	tileMins.y = spriteY * m_tileDimensions.y;

	Vector2 tileMaxs;
	tileMaxs.x = tileMins.x + m_tileDimensions.x;
	tileMaxs.y = tileMins.y + m_tileDimensions.y;

	return AABB2(tileMins, tileMaxs);
}

AABB2 SpriteSheet::GetTexCoordsForSpriteCoords(const IntVector2& spriteCoords) const{
	return GetTexCoordsForSpriteCoords(spriteCoords.x, spriteCoords.y);
}

AABB2 SpriteSheet::GetTexCoordsForSpriteIndex(int spriteIndex) const{
	int spriteX = spriteIndex % m_spriteLayout.x;
	int spriteY = spriteIndex / m_spriteLayout.x;
	return GetTexCoordsForSpriteCoords(spriteX, spriteY);
}

int SpriteSheet::GetNumSprites() const{
	return m_spriteLayout.x * m_spriteLayout.y;
}

RHITexture2D& SpriteSheet::GetTexture() const{
	return m_spriteSheetTexture;
}