#pragma once

#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Vertex2.hpp"
#include "Engine/Renderer/Vertex3.hpp"
#include "Engine/Renderer/Texture.hpp"

#include <string>
#include <vector>

enum PrimitiveType{
	PRIMITIVE_TYPE_POINTS,
	PRIMITIVE_TYPE_LINES,
	PRIMITIVE_TYPE_LINE_LOOP,
	PRIMITIVE_TYPE_TRIANGLES,
	PRIMITIVE_TYPE_TRIANGLE_STRIP,
	PRIMITIVE_TYPE_TRIANGLE_FAN,
	PRIMITIVE_TYPE_QUADS,
	PRIMITIVE_TYPE_QUAD_STRIP,
	PRIMITIVE_TYPE_POLYGON,
	NUM_PRIMITIVE_TYPES
};

class BitmapFont;

class Renderer{
public:
	Renderer();
	~Renderer();

	void Init();

	void ClearScreen(const Rgba& clearColor = Rgba::BLACK);
	void ClearDepth();
	void SetOrthoProjection(const Vector2& bottomLeft, const Vector2& topRight);
	void SetOrthoProjection(const AABB2& bounds);
	void SetPerspectiveProjection(float fovVerticalDegrees, float aspectRatio, float nearDistance, float farDistance);
	void SetViewport(int x, int y, int width, int height);

	void EnableModelViewMode();

	void SetDrawColor(const Rgba& drawColor);
	void SetAdditiveBlending();
	void SetAlphaBlending();

	void EnableBackFaceCulling(bool isNowBackFaceCulling = true);
	void EnableDepthTesting(bool isNowDepthTesting = true);

	void PushMatrix();
	void PopMatrix();
	void LoadMatrix(const Matrix4& matrixToLoad);

	void Translate3D(const IntVector3& position);
	void Translate3D(const Vector3& position);
	void Translate3D(float translateX, float translateY, float translateZ);
	void RotateDegrees3D(float angleDegrees, const Vector3& axisOfRotation);

	void Translate2D(const IntVector2& position);
	void Translate2D(const Vector2& position);
	void Translate2D(float translateX, float translateY);
	void RotateDegrees2D(float angleDegrees);
	void Scale2D(float scaleX, float scaleY);
	void Scale3D(const Vector3& scaleFactors);
	void Scale3D(float scaleX, float scaleY, float scaleZ);

	void DrawPoint2D(const Vector2& position, const Rgba& drawColor = Rgba::WHITE, float pointSize = 2.f);
	void DrawPoint2D(const Vertex2& vertex, float pointSize = 2.f);

	void DrawPoint3D(const Vector3& position, const Rgba& drawColor = Rgba::WHITE, float pointSize = 2.f, bool enableDepthTesting = false);

	void DrawLine2D(float startX, float startY, float endX, float endY, const Rgba& startColor = Rgba::WHITE, const Rgba& endColor = Rgba::WHITE, float lineWidth = 2.f);
	void DrawLine2D(const Vector2& start, const Vector2& end, const Rgba& startColor = Rgba::WHITE, const Rgba& endColor = Rgba::WHITE, float lineWidth = 2.f);
	void DrawLine2D(const Vertex2& startVertex, const Vertex2& endVertex, float lineWidth = 2.f);
	void DrawLine2D(const LineSegment2& lineSegment, const Rgba& drawColor = Rgba::WHITE, float lineWidth = 2.f);

	void DrawLine3D(const Vector3& start, const Vector3& end, const Rgba& startColor = Rgba::WHITE, const Rgba& endColor = Rgba::WHITE, float lineWidth = 2.f);

	void DrawCircle2D(float centerX, float centerY, float radius, const Rgba& color = Rgba::WHITE, float numSides = 64.f, float lineWidth = 2.f);
	void DrawCircle2D(const Vector2& position, float radius, const Rgba& color = Rgba::WHITE, float numSides = 64.f, float lineWidth = 2.f);
	void DrawCircle2D(const Disc2& disc, const Rgba& drawColor = Rgba::WHITE, float numSides = 64.f, float lineWidth = 2.f);

	void DrawLineLoop2D(const Vertex2* vertices, int numVertices, float lineWidth = 2.f);
	void DrawLineLoop2D(const Vector2* positions, int numPositions, const Rgba& drawColor = Rgba::WHITE, float lineWidth = 2.f);

	void DrawLineLoop3D(const Vector3* positions, int numPositions, const Rgba& drawColor = Rgba::WHITE, float lineWidth = 2.f);

	void DrawDisc2D(const Vector2& center, float radius, const Rgba& centerColor = Rgba::WHITE, const Rgba& outerColor = Rgba::WHITE);
	void DrawDisc2D(const Disc2& disc, const Rgba& centerColor = Rgba::WHITE, const Rgba& outerColor = Rgba::WHITE);

	void DrawAABB2D(const AABB2& bounds, const Rgba& drawColor = Rgba::WHITE);
	void DrawAABB2D(const Vector2& mins, const Vector2& maxs, const Rgba& drawColor = Rgba::WHITE);
	void DrawBox2D(const AABB2& bounds, const Rgba& lineColor = Rgba::WHITE, float lineWidth = 2.f);
	void DrawBox2D(const Vector2& mins, const Vector2& maxs, const Rgba& lineColor = Rgba::WHITE, float lineWidth = 2.f);

	void DrawQuad2D(const Vertex2& topLeft, const Vertex2& topRight, const Vertex2& bottomRight, const Vertex2& bottomLeft);
	void DrawQuad3D(const Vertex3& topLeft, const Vertex3& topRight, const Vertex3& bottomRight, const Vertex3& bottomLeft);

	void DrawCapsule2D(const Capsule2& capsule, const Rgba& drawColor = Rgba::WHITE);

	void DrawTexturedAABB2D(const AABB2& bounds, const Texture& texture, const Vector2& texCoordsAtMins = Vector2::ZERO, const Vector2& texCoordsAtMaxs = Vector2::ONE, const Rgba& tint = Rgba::WHITE);
	void DrawTexturedAABB2D(const Vector2& mins, const Vector2& maxs, const Texture& texture, const Vector2& texCoordsAtMins = Vector2::ZERO, const Vector2& texCoordsAtMaxs = Vector2::ONE, const Rgba& tint = Rgba::WHITE);

	void DrawAABB3DEdges(const AABB3& bounds, const Rgba& color = Rgba::WHITE, float lineWidth = 2.f);

	BitmapFont* CreateOrGetFont(const std::string& fontName);
	void SetDefaultFont(const std::string& fontName);
	void DrawText2D(const Vector2& startBottomLeft, const std::string& asciiText, float cellHeight, float cellAspectScale = 1.0f, const Rgba& tint = Rgba::WHITE, const BitmapFont* font = nullptr);

	Texture* CreateOrGetTexture(const std::string& imageFilePath);
	void BindTexture(const Texture* texture);

	void DrawAxes(float axisLength, float lineWidth = 1.0f, float lineAlphaScale = 1.0f);

	unsigned int CreateVBO();
	void UpdateVBO(unsigned int vboID, const Vertex3* vertexes, int numVertexes);
	void DestroyVBO(unsigned int vboID);

	void DrawVertexes_VA(const Vertex3* vertexes, int numVertexes, PrimitiveType primitiveType = PRIMITIVE_TYPE_QUADS);
	void DrawVertexes_VBO(unsigned int vboID, int numVertexes, PrimitiveType primitiveType = PRIMITIVE_TYPE_QUADS);

private:
	std::vector<Texture*> m_alreadyLoadedTextures;

	Texture* CreateTextureFromFile( const std::string& imageFilePath );
	Texture* GetTexture( const std::string& imageFilePath );
	unsigned int CreateOpenGLTexture( unsigned char* imageTexelBytes, int width, int height, int bytesPerTexel );

	BitmapFont* m_currentDefaultFont;
	std::vector<BitmapFont*> m_loadedFonts;
	BitmapFont* GetFont(const std::string& fontName);

	const Texture* m_currentTexture;

	void LoadDefaultTextures();
	Texture* m_defaultTexture;
};