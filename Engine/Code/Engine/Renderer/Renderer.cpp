#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "glu32" ) // Link in the glu32.lib static library

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Common.hpp"

#include "ThirdParty/stb/stb_image.h"

Renderer::Renderer()
	:m_currentTexture(nullptr),
	 m_currentDefaultFont(nullptr)
{
	Init();
};

Renderer::~Renderer(){
}

void Renderer::Init(){
	OpenGL::InitExtensions();
	LoadDefaultTextures();
	SetAlphaBlending();
	glEnable(GL_LINE_SMOOTH);
}

void Renderer::ClearScreen(const Rgba& clearColor){
	float r, g, b, a;
	clearColor.GetAsFloats(r, g, b, a);

	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::ClearDepth(){
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::SetOrthoProjection(const Vector2& bottomLeft, const Vector2& topRight){
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.0f, 1.0f);
}

void Renderer::SetOrthoProjection(const AABB2& bounds){
	SetOrthoProjection(bounds.mins, bounds.maxs);
}

void Renderer::SetPerspectiveProjection(float fovVerticalDegrees, float aspectRatio, float nearDistance, float farDistance){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovVerticalDegrees, aspectRatio, nearDistance, farDistance);
}

void Renderer::SetViewport(int x, int y, int width, int height){
	glViewport(x, y, width, height);
}

void Renderer::EnableModelViewMode(){
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Renderer::SetDrawColor(const Rgba& drawColor){
	glColor4ub(drawColor.r, drawColor.g, drawColor.b, drawColor.a);
}

void Renderer::SetAdditiveBlending(){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void Renderer::SetAlphaBlending(){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::EnableBackFaceCulling(bool isNowBackFaceCulling){
	if(isNowBackFaceCulling){
		glEnable(GL_CULL_FACE);
	} else{
		glDisable(GL_CULL_FACE);
	}
}

void Renderer::EnableDepthTesting(bool isNowDepthTesting){
	if(isNowDepthTesting){
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	} else{
		glDisable(GL_DEPTH_TEST);
	}
}

void Renderer::PushMatrix(){
	glPushMatrix();
}

void Renderer::PopMatrix(){
	glPopMatrix();
}

void Renderer::LoadMatrix(const Matrix4& matrixToLoad){
	glLoadMatrixf(matrixToLoad.data);
}

void Renderer::Translate3D(const IntVector3& position){
	Translate3D((float)position.x, (float)position.y, (float)position.z);
}

void Renderer::Translate3D(const Vector3& position){
	glTranslatef(position.x, position.y, position.z);
}

void Renderer::Translate3D(float translateX, float translateY, float translateZ){
	Translate3D(Vector3(translateX, translateY, translateZ));
}

void Renderer::RotateDegrees3D(float angleDegrees, const Vector3& axisOfRotation){
	glRotatef(angleDegrees, axisOfRotation.x, axisOfRotation.y, axisOfRotation.z);
}

void Renderer::Translate2D(const IntVector2& position){
	Translate2D((float)position.x, (float)position.y);
}

void Renderer::Translate2D(const Vector2& position){
	glTranslatef(position.x, position.y, 0.f);
}

void Renderer::Translate2D(float translateX, float translateY){
	glTranslatef(translateX, translateY, 0.f);
}

void Renderer::RotateDegrees2D(float angleDegrees){
	glRotatef(angleDegrees, 0.f, 0.f, 1.f);
}

void Renderer::Scale2D(float scaleX, float scaleY){
	glScalef(scaleX, scaleY, 0.f);
}

void Renderer::Scale3D(const Vector3& scaleFactors){
	glScalef(scaleFactors.x, scaleFactors.y, scaleFactors.z);
}

void Renderer::Scale3D(float scaleX, float scaleY, float scaleZ){
	Scale3D(Vector3(scaleX, scaleY, scaleZ));
}

void Renderer::DrawPoint2D(const Vector2& position, const Rgba& drawColor, float pointSize){
	BindTexture(nullptr);

	Vector3 position3D(position.x, position.y, 0);
	Vertex3 vertex(position3D, drawColor);

	glPointSize(pointSize);
	DrawVertexes_VA(&vertex, 1, PRIMITIVE_TYPE_POINTS);
}

void Renderer::DrawPoint2D(const Vertex2& vertex, float pointSize){
	DrawPoint2D(vertex.m_position, vertex.m_color, pointSize);
}

void Renderer::DrawPoint3D(const Vector3& position, const Rgba& drawColor, float pointSize, bool enableDepthTesting){
	BindTexture(nullptr);

	EnableDepthTesting(enableDepthTesting);

	Vertex3 vertex(position, drawColor);

	glPointSize(pointSize);
	DrawVertexes_VA(&vertex, 1, PRIMITIVE_TYPE_POINTS);
}

void Renderer::DrawLine2D(float startX, float startY, float endX, float endY, const Rgba& startColor, const Rgba& endColor, float lineWidth){
	BindTexture(nullptr);

	Vertex3 vertexes[2];

	vertexes[0].m_position = Vector3(startX, startY, 0.f);
	vertexes[0].m_color = startColor;

	vertexes[1].m_position = Vector3(endX, endY, 0.f);
	vertexes[1].m_color = endColor;

	glLineWidth(lineWidth);
	DrawVertexes_VA(vertexes, 2, PRIMITIVE_TYPE_LINES);
}

void Renderer::DrawLine2D(const Vector2& start, const Vector2& end, const Rgba& startColor, const Rgba& endColor, float lineWidth){
	DrawLine2D(start.x, start.y, end.x, end.y, startColor, endColor, lineWidth);
}

void Renderer::DrawLine2D(const LineSegment2& lineSegment, const Rgba& drawColor, float lineWidth){
	DrawLine2D(lineSegment.start, lineSegment.end, drawColor, drawColor, lineWidth);
}

void Renderer::DrawLine2D(const Vertex2& startVertex, const Vertex2& endVertex, float lineWidth){
	DrawLine2D(startVertex.m_position.x, startVertex.m_position.y, endVertex.m_position.x, endVertex.m_position.y, startVertex.m_color, endVertex.m_color, lineWidth);
}

void Renderer::DrawLine3D(const Vector3& start, const Vector3& end, const Rgba& startColor, const Rgba& endColor, float lineWidth){
	BindTexture(nullptr);

	Vertex3 vertexes[2];

	vertexes[0].m_position = start;
	vertexes[0].m_color = startColor;

	vertexes[1].m_position = end;
	vertexes[1].m_color = endColor;

	glLineWidth(lineWidth);
	DrawVertexes_VA(vertexes, 2, PRIMITIVE_TYPE_LINES);
}

void Renderer::DrawCircle2D(float centerX, float centerY, float radius, const Rgba& drawColor, float numSides, float lineWidth){
	BindTexture(nullptr);

	std::vector<Vertex3> vertexes;

	float degreesPerVertex = 360.f / numSides;
	for(float degrees = 0.0f; degrees <= 360.f; degrees += degreesPerVertex){
		float xPos = centerX + (radius * CosDegrees(degrees));
		float yPos = centerY + (radius * SinDegrees(degrees));
		
		Vertex3 vertex;
		vertex.m_position = Vector3(xPos, yPos, 0.f);
		vertex.m_color = drawColor;

		vertexes.push_back(vertex);
	}

	glLineWidth(lineWidth);
	DrawVertexes_VA(vertexes.data(), vertexes.size(), PRIMITIVE_TYPE_LINE_LOOP);
}

void Renderer::DrawCircle2D(const Vector2& position, float radius, const Rgba& color, float numSides, float lineWidth){
	DrawCircle2D(position.x, position.y, radius, color, numSides, lineWidth);
}

void Renderer::DrawCircle2D(const Disc2& disc, const Rgba& drawColor, float numSides, float lineWidth){
	DrawCircle2D(disc.center.x, disc.center.y, disc.radius, drawColor, numSides, lineWidth);
}

void Renderer::DrawLineLoop2D(const Vertex2* vertexes, int numVertexes, float lineWidth){
	BindTexture(nullptr);

	std::vector<Vertex3> vertexes3D;
	vertexes3D.resize(numVertexes);

	for(int vertexIndex = 0; vertexIndex < numVertexes; ++vertexIndex){
		Vector2 position2D = vertexes[vertexIndex].m_position;
		Vector3 position3D(position2D.x, position2D.y, 0.0f);

		vertexes3D[vertexIndex].m_position = position3D;
		vertexes3D[vertexIndex].m_color = vertexes[vertexIndex].m_color;
	}

	glLineWidth(lineWidth);
	DrawVertexes_VA(vertexes3D.data(), vertexes3D.size(), PRIMITIVE_TYPE_LINE_LOOP);
}

void Renderer::DrawLineLoop2D(const Vector2* positions, int numPositions, const Rgba& drawColor, float lineWidth){
	BindTexture(nullptr);

	std::vector<Vertex3> vertexes;
	vertexes.resize(numPositions);

	for(int vertexIndex = 0; vertexIndex < numPositions; ++vertexIndex){
		Vector2 position = positions[vertexIndex];
		vertexes[vertexIndex].m_position = Vector3(position.x, position.y, 0.0f);
		vertexes[vertexIndex].m_color = drawColor;
	}

	glLineWidth(lineWidth);
	DrawVertexes_VA(vertexes.data(), vertexes.size(), PRIMITIVE_TYPE_LINE_LOOP);
}

void Renderer::DrawLineLoop3D(const Vector3* positions, int numPositions, const Rgba& drawColor, float lineWidth){
	BindTexture(nullptr);

	std::vector<Vertex3> vertexes;
	vertexes.resize(numPositions);

	for(int vertexIndex = 0; vertexIndex < numPositions; ++vertexIndex){
		vertexes[vertexIndex].m_position = positions[vertexIndex];
		vertexes[vertexIndex].m_color = drawColor;
	}

	glLineWidth(lineWidth);
	DrawVertexes_VA(vertexes.data(), vertexes.size(), PRIMITIVE_TYPE_LINE_LOOP);
}

void Renderer::DrawDisc2D(const Vector2& center, float radius, const Rgba& centerColor, const Rgba& outerColor){
	BindTexture(nullptr);

	std::vector<Vertex3> vertexes;

	Vertex3 vertex;
	vertex.m_position = Vector3(center.x, center.y, 0.0f);
	vertex.m_color = centerColor;
	vertexes.push_back(vertex);

	float degreesPerVertex = 360.f / 64.f;
	for(float degrees = 0.f; degrees <= 360.f; degrees += degreesPerVertex){
		float x = center.x + (radius * CosDegrees(degrees));
		float y = center.y + (radius * SinDegrees(degrees));

		vertex.m_position = Vector3(x, y, 0.0f);
		vertex.m_color = outerColor;
		vertexes.push_back(vertex);
	}

	DrawVertexes_VA(vertexes.data(), vertexes.size(), PRIMITIVE_TYPE_TRIANGLE_FAN);
}

void Renderer::DrawDisc2D(const Disc2& disc, const Rgba& centerColor, const Rgba& outerColor){
	DrawDisc2D(disc.center, disc.radius, centerColor, outerColor);
}

void Renderer::DrawQuad2D(const Vertex2& topLeft, const Vertex2& topRight, const Vertex2& bottomRight, const Vertex2& bottomLeft){
	BindTexture(nullptr);

	Vertex3 vertexes[4];

	vertexes[0].m_position = Vector3(topLeft.m_position.x, topLeft.m_position.y, 0.0f);
	vertexes[0].m_color	= topLeft.m_color;

	vertexes[1].m_position = Vector3(topRight.m_position.x, topRight.m_position.y, 0.0f);
	vertexes[1].m_color	= topRight.m_color;

	vertexes[2].m_position = Vector3(bottomRight.m_position.x, bottomRight.m_position.y, 0.0f);
	vertexes[2].m_color	= bottomRight.m_color;

	vertexes[3].m_position = Vector3(bottomLeft.m_position.x, bottomLeft.m_position.y, 0.0f);
	vertexes[3].m_color	= bottomLeft.m_color;

	DrawVertexes_VA(vertexes, 4, PRIMITIVE_TYPE_QUADS);
}

void Renderer::DrawQuad3D(const Vertex3& topLeft, const Vertex3& topRight, const Vertex3& bottomRight, const Vertex3& bottomLeft){
	Vertex3 vertexes[4] = { topRight, topLeft, bottomLeft, bottomRight };
	DrawVertexes_VA(vertexes, 4, PRIMITIVE_TYPE_QUADS);
}

void Renderer::DrawAABB2D(const AABB2& bounds, const Rgba& drawColor){
	BindTexture(nullptr);

	Vertex3 vertexes[4];

	vertexes[0].m_position = Vector3(bounds.mins.x, bounds.maxs.y, 0.0f);
	vertexes[0].m_color = drawColor;

	vertexes[1].m_position = Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f);
	vertexes[1].m_color = drawColor;

	vertexes[2].m_position = Vector3(bounds.maxs.x, bounds.mins.y, 0.0f);
	vertexes[2].m_color = drawColor;

	vertexes[3].m_position = Vector3(bounds.mins.x, bounds.mins.y, 0.0f);
	vertexes[3].m_color = drawColor;

	DrawVertexes_VA(vertexes, 4, PRIMITIVE_TYPE_QUADS);
}

void Renderer::DrawAABB2D(const Vector2& mins, const Vector2& maxs, const Rgba& drawColor){
	DrawAABB2D(AABB2(mins, maxs), drawColor);
}

void Renderer::DrawBox2D(const AABB2& bounds, const Rgba& lineColor, float lineWidth){
	DrawLine2D(bounds.mins, bounds.CalcTopLeft(), lineColor, lineColor, lineWidth);
	DrawLine2D(bounds.mins, bounds.CalcBottomRight(), lineColor, lineColor, lineWidth);
	DrawLine2D(bounds.maxs, bounds.CalcTopLeft(), lineColor, lineColor, lineWidth);
	DrawLine2D(bounds.maxs, bounds.CalcBottomRight(), lineColor, lineColor, lineWidth);
}

void Renderer::DrawBox2D(const Vector2& mins, const Vector2& maxs, const Rgba& lineColor, float lineWidth){
	DrawBox2D(AABB2(mins, maxs), lineColor, lineWidth);
}

void Renderer::DrawCapsule2D(const Capsule2& capsule, const Rgba& drawColor){
	DrawDisc2D(capsule.line.start, capsule.radius, drawColor, drawColor);
	DrawDisc2D(capsule.line.end, capsule.radius, drawColor, drawColor);

	PushMatrix();

	Translate2D(capsule.line.start);

	Vector2 capsuleDirection = capsule.line.CalcDirection();
	RotateDegrees2D(capsuleDirection.CalcHeadingDegrees());

	Vector2 minPos(0.f, -capsule.radius);
	Vector2 maxPos(capsule.line.CalcLength(), capsule.radius);

	DrawAABB2D(AABB2(minPos, maxPos), drawColor);

	PopMatrix();
}

// Based on code by Professor Squirrel Eiserloh
Texture* Renderer::CreateOrGetTexture(const std::string& imageFilePath){
	// Try to find that texture from those already loaded
	Texture* texture = GetTexture( imageFilePath );
	if( texture )
		return texture;

	texture = CreateTextureFromFile( imageFilePath );
	return texture;
}

void Renderer::BindTexture(const Texture* texture){
	if(texture && m_currentTexture != texture){
		glBindTexture(GL_TEXTURE_2D, texture->m_textureID);
		m_currentTexture = texture;
	} else if(texture == nullptr){
		glBindTexture(GL_TEXTURE_2D, m_defaultTexture->m_textureID);
		m_currentTexture = nullptr;
	}
}

// Based on code by Professor Squirrel Eiserloh
void Renderer::DrawTexturedAABB2D(const AABB2& bounds, const Texture& texture, const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint){
	BindTexture(&texture);

	Vertex3 vertexes[4];

	vertexes[0].m_position = Vector3(bounds.mins.x, bounds.mins.y, 0.0f);
	vertexes[0].m_color = tint;
	vertexes[0].m_texCoords = Vector2(texCoordsAtMins.x, texCoordsAtMaxs.y);

	vertexes[1].m_position = Vector3(bounds.maxs.x, bounds.mins.y, 0.0f);
	vertexes[1].m_color = tint;
	vertexes[1].m_texCoords = Vector2(texCoordsAtMaxs.x, texCoordsAtMaxs.y);

	vertexes[2].m_position = Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f);
	vertexes[2].m_color = tint;
	vertexes[2].m_texCoords = Vector2(texCoordsAtMaxs.x, texCoordsAtMins.y);

	vertexes[3].m_position = Vector3(bounds.mins.x, bounds.maxs.y, 0.0f);
	vertexes[3].m_color = tint;
	vertexes[3].m_texCoords = Vector2(texCoordsAtMins.x, texCoordsAtMins.y);

	DrawVertexes_VA(vertexes, 4, PRIMITIVE_TYPE_QUADS);
}

void Renderer::DrawTexturedAABB2D(const Vector2& mins, const Vector2& maxs, const Texture& texture, const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint){
	DrawTexturedAABB2D(AABB2(mins, maxs), texture, texCoordsAtMins, texCoordsAtMaxs, tint);
}

void Renderer::DrawAABB3DEdges(const AABB3& bounds, const Rgba& color, float lineWidth){
	Vector3 mins = bounds.mins;
	Vector3 maxs = bounds.maxs;

	// Bottom
	DrawLine3D(Vector3(mins.x, mins.y, mins.z), Vector3(maxs.x, mins.y, mins.z), color, color, lineWidth);
	DrawLine3D(Vector3(maxs.x, mins.y, mins.z), Vector3(maxs.x, maxs.y, mins.z), color, color, lineWidth);
	DrawLine3D(Vector3(maxs.x, maxs.y, mins.z), Vector3(mins.x, maxs.y, mins.z), color, color, lineWidth);
	DrawLine3D(Vector3(mins.x, maxs.y, mins.z), Vector3(mins.x, mins.y, mins.z), color, color, lineWidth);

	// Top
	DrawLine3D(Vector3(mins.x, mins.y, maxs.z), Vector3(maxs.x, mins.y, maxs.z), color, color, lineWidth);
	DrawLine3D(Vector3(maxs.x, mins.y, maxs.z), Vector3(maxs.x, maxs.y, maxs.z), color, color, lineWidth);
	DrawLine3D(Vector3(maxs.x, maxs.y, maxs.z), Vector3(mins.x, maxs.y, maxs.z), color, color, lineWidth);
	DrawLine3D(Vector3(mins.x, maxs.y, maxs.z), Vector3(mins.x, mins.y, maxs.z), color, color, lineWidth);

	// Min-X side
	DrawLine3D(Vector3(mins.x, maxs.y, mins.z), Vector3(mins.x, maxs.y, maxs.z), color, color, lineWidth);
	DrawLine3D(Vector3(mins.x, mins.y, maxs.z), Vector3(mins.x, mins.y, mins.z), color, color, lineWidth);

	// Max-X side
	DrawLine3D(Vector3(maxs.x, maxs.y, mins.z), Vector3(maxs.x, maxs.y, maxs.z), color, color, lineWidth);
	DrawLine3D(Vector3(maxs.x, mins.y, maxs.z), Vector3(maxs.x, mins.y, mins.z), color, color, lineWidth);
}

BitmapFont* Renderer::CreateOrGetFont(const std::string& fontName){
	UNUSED(fontName);
	return nullptr;
	//BitmapFont* font = GetFont(fontName);
	//if(font)
	//	return font;

	//std::string fontFilePath = Stringf("Data/Fonts/%s", fontName.c_str());

	//Texture* glyphTexture = CreateOrGetTexture(fontFilePath);
	//font = new BitmapFont(fontName, *glyphTexture);
	//m_loadedFonts.push_back(font);

	//return font;
}

void Renderer::SetDefaultFont(const std::string& fontName){
	m_currentDefaultFont = CreateOrGetFont(fontName);
}

void Renderer::DrawText2D(const Vector2& startBottomLeft, const std::string& asciiText, float cellHeight, float cellAspectScale, const Rgba&tint, const BitmapFont* font){
	UNUSED(startBottomLeft);
	UNUSED(asciiText);
	UNUSED(cellHeight);
	UNUSED(cellAspectScale);
	UNUSED(tint);
	UNUSED(font);

	//if(!font && m_currentDefaultFont){
	//	font = m_currentDefaultFont;
	//}

	//float charCellWidth = cellHeight * cellAspectScale;

	//Vector2 charCellMins = startBottomLeft;
	//Vector2 charCellMaxs = charCellMins + Vector2(charCellWidth, cellHeight);

	//for(unsigned int charIndex = 0; charIndex < asciiText.size(); ++charIndex){
	//	AABB2 glyphTexCoords = font->GetGlyphTextCoordsForGlyph(asciiText[charIndex]);
	//	DrawTexturedAABB2D(charCellMins, charCellMaxs, font->m_glyphTexture, glyphTexCoords.mins, glyphTexCoords.maxs, tint);

	//	charCellMins.x += charCellWidth;
	//	charCellMaxs.x += charCellWidth;
	//}
}

void Renderer::DrawAxes(float axisLength, float lineWidth, float lineAlphaScale){
	BindTexture(nullptr);

	Vertex3 vertexes[6];

	Rgba xAxisColor = Rgba::RED;
	xAxisColor.ScaleAlpha(lineAlphaScale);

	// Draw x-axis
	vertexes[0].m_position = Vector3::ZERO;
	vertexes[0].m_color = xAxisColor;
	vertexes[0].m_texCoords = Vector2::ZERO;

	vertexes[1].m_position = Vector3(axisLength, 0.0f, 0.0f);
	vertexes[1].m_color = xAxisColor;
	vertexes[1].m_texCoords = Vector2::ZERO;

	// Draw y-axis
	Rgba yAxisColor = Rgba::GREEN;
	yAxisColor.ScaleAlpha(lineAlphaScale);

	vertexes[2].m_position = Vector3::ZERO;
	vertexes[2].m_color = yAxisColor;
	vertexes[2].m_texCoords = Vector2::ZERO;

	vertexes[3].m_position = Vector3(0.0f, axisLength, 0.0f);
	vertexes[3].m_color = yAxisColor;
	vertexes[3].m_texCoords = Vector2::ZERO;


	// Draw z-axis
	Rgba zAxisColor = Rgba::BLUE;
	zAxisColor.ScaleAlpha(lineAlphaScale);

	vertexes[4].m_position = Vector3::ZERO;
	vertexes[4].m_color = zAxisColor;
	vertexes[4].m_texCoords = Vector2::ZERO;

	vertexes[5].m_position = Vector3(0.0f, 0.0f, axisLength);
	vertexes[5].m_color = zAxisColor;
	vertexes[5].m_texCoords = Vector2::ZERO;

	glLineWidth(lineWidth);
	DrawVertexes_VA(vertexes, 6, PRIMITIVE_TYPE_LINES);
}

unsigned int Renderer::CreateVBO(){
	unsigned int vboID;
	glGenBuffers(1, &vboID);
	return vboID;
}

void Renderer::UpdateVBO(unsigned int vboID, const Vertex3* vertexes, int numVertexes){
	size_t vertexArrayNumBytes = sizeof(Vertex3) * numVertexes;
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, vertexArrayNumBytes, vertexes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::DestroyVBO(unsigned int vboID){
	glDeleteBuffers(1, &vboID);
}

GLenum GetGLDrawModeForPrimitiveType(PrimitiveType primitiveType){
	GLenum glDrawMode;

	switch(primitiveType){
	case PRIMITIVE_TYPE_POINTS:
		glDrawMode = GL_POINTS;
		break;
	case PRIMITIVE_TYPE_LINES:
		glDrawMode = GL_LINES;
		break;
	case PRIMITIVE_TYPE_LINE_LOOP:
		glDrawMode = GL_LINE_LOOP;
		break;
	case PRIMITIVE_TYPE_TRIANGLES:
		glDrawMode = GL_TRIANGLES;
		break;
	case PRIMITIVE_TYPE_TRIANGLE_STRIP:
		glDrawMode = GL_TRIANGLE_STRIP;
		break;
	case PRIMITIVE_TYPE_TRIANGLE_FAN:
		glDrawMode = GL_TRIANGLE_FAN;
		break;
	case PRIMITIVE_TYPE_QUADS:
		glDrawMode = GL_QUADS;
		break;
	case PRIMITIVE_TYPE_QUAD_STRIP:
		glDrawMode = GL_QUAD_STRIP;
		break;
	case PRIMITIVE_TYPE_POLYGON:
		glDrawMode = GL_POLYGON;
		break;
	default:
		glDrawMode = GL_QUADS;
		break;
	}

	return glDrawMode;
}


void Renderer::DrawVertexes_VA(const Vertex3* vertexes, int numVertexes, PrimitiveType primitiveType){
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	constexpr int stride = sizeof(Vertex3);

	glVertexPointer(	3,		GL_FLOAT,			stride,		&vertexes[0].m_position);
	glColorPointer(		4,		GL_UNSIGNED_BYTE,	stride,		&vertexes[0].m_color);
	glTexCoordPointer(	2,		GL_FLOAT,			stride,		&vertexes[0].m_texCoords);

	GLenum glDrawMode = GetGLDrawModeForPrimitiveType(primitiveType);

	glDrawArrays(glDrawMode, 0, numVertexes);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Renderer::DrawVertexes_VBO(unsigned int vboID, int numVertexes, PrimitiveType primitiveType){
	glBindBuffer(GL_ARRAY_BUFFER, vboID);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	constexpr int stride = sizeof(Vertex3);

	glVertexPointer(	3,		GL_FLOAT,			stride,		(const GLvoid*)offsetof(Vertex3, m_position));
	glColorPointer(		4,		GL_UNSIGNED_BYTE,	stride,		(const GLvoid*)offsetof(Vertex3, m_color));
	glTexCoordPointer(	2,		GL_FLOAT,			stride,		(const GLvoid*)offsetof(Vertex3, m_texCoords));

	GLenum glDrawMode = GetGLDrawModeForPrimitiveType(primitiveType);

	glDrawArrays(glDrawMode, 0, numVertexes);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Based on code by Professor Squirrel Eiserloh
Texture* Renderer::CreateTextureFromFile(const std::string& imageFilePath){
	// Load image data
	int width = 0;
	int height = 0;
	int bytesPerTexel = 0;
	unsigned char* imageTexelBytes = stbi_load( imageFilePath.c_str(), &width, &height, &bytesPerTexel, 0 );
	GUARANTEE_OR_DIE( imageTexelBytes != nullptr, Stringf( "Failed to load image file \"%s\" - file not found!", imageFilePath.c_str() ) );
	GUARANTEE_OR_DIE( bytesPerTexel == 3 || bytesPerTexel == 4, Stringf( "Failed to load image file \"%s\" - image had unsupported %i bytes per texel (must be 3 or 4)", imageFilePath.c_str(), bytesPerTexel ) );

	// Create texture on video card, and send image (texel) data
	unsigned int openGLTextureID = CreateOpenGLTexture( imageTexelBytes, width, height, bytesPerTexel );
	stbi_image_free( imageTexelBytes );

	// Create (new) a Texture object
	Texture* texture = new Texture();
	texture->m_textureID = openGLTextureID;
	texture->m_imageFilePath = imageFilePath;
	texture->m_texelDimensions.SetXY( width, height );

	m_alreadyLoadedTextures.push_back( texture );
	return texture;
}

// Based on code by Professor Squirrel Eiserloh
Texture* Renderer::GetTexture(const std::string& imageFilePath){
	for( int textureIndex = 0; textureIndex < (int) m_alreadyLoadedTextures.size(); ++ textureIndex )
	{
		Texture* texture = m_alreadyLoadedTextures[ textureIndex ];
		if( imageFilePath == texture->m_imageFilePath )
			return texture;
	}

	return nullptr;
}

// Based on code by Professor Squirrel Eiserloh
unsigned int Renderer::CreateOpenGLTexture(unsigned char* imageTexelBytes, int width, int height, int bytesPerTexel){
	// Create a texture ID (or "name" as OpenGL calls it) for this new texture
	unsigned int openGLTextureID = 0xFFFFFFFF;
	glGenTextures( 1, &openGLTextureID );

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, openGLTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // one of: GL_CLAMP or GL_REPEAT
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // one of: GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if( bytesPerTexel == 3 )
		bufferFormat = GL_RGB;

	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		width,				// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		height,				// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageTexelBytes );	// Location of the actual pixel data bytes/buffer

	return openGLTextureID;
}

BitmapFont* Renderer::GetFont(const std::string& fontName){
	for(BitmapFont* font : m_loadedFonts){
		if(font->m_fontName == fontName)
			return font;
	}

	return nullptr;
}

void Renderer::LoadDefaultTextures(){
	unsigned char whiteBytes[] = { 255, 255, 255, 255, };

	unsigned int textureID = CreateOpenGLTexture(whiteBytes, 1, 1, 4);

	Texture* texture = new Texture();
	texture->m_textureID = textureID;
	texture->m_imageFilePath = "";
	texture->m_texelDimensions.SetXY(1, 1);

	m_defaultTexture = texture;
}