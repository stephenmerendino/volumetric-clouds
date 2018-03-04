#pragma once

#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"

class SpriteSheet{
public:
	SpriteSheet(RHITexture2D& texture, const IntVector2& spriteSheetLayout);
	SpriteSheet(RHITexture2D& texture, int spriteSheetLayoutWidth, int spriteSheetLayoutHeight);

	AABB2 GetTexCoordsForSpriteCoords(int spriteX, int spriteY) const;
	AABB2 GetTexCoordsForSpriteCoords(const IntVector2& spriteCoords) const;
	AABB2 GetTexCoordsForSpriteIndex(int spriteIndex) const;
	int GetNumSprites() const;
	RHITexture2D& GetTexture() const;

private:
	RHITexture2D&	m_spriteSheetTexture;
	IntVector2		m_spriteLayout;
	Vector2			m_tileDimensions;
};