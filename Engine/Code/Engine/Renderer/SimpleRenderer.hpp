#pragma once

class RHIDevice;
class RHIDeviceContext;
class RHIOutput;
class RHITextureBase;
class RHITexture2D;
class ShaderProgram;
class Shader;
class Sampler;
class VertexBuffer;
class IndexBuffer;
class Window;
class RasterState;
class BlendState;
class ConstantBuffer;
class StructuredBuffer;
class Vertex3;
class Font;
class Material;
class Mesh;
class Skeleton;
class SkeletonInstance;
class ComputeJob;

#include "Engine/Core/Rgba.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/Renderer/CubeMap.hpp"
#include "Engine/Renderer/SkeletalTransformHierarchy.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/transform.h"
#include "Engine/Renderer/scene.h"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

#define MATRIX_BUFFER_INDEX (0)
#define TIME_BUFFER_INDEX (1)
#define LIGHT_BUFFER_INDEX (2)
#define MIDI_BUFFER_INDEX (3)

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16

// --------------------------------------------------------------------
// Institutionalized Constant Buffers that are used in every shader
// --------------------------------------------------------------------

// --------------------------------------------------------------------
struct TimeBufferData
{
	float gameTime;
	float systemTime;
	float gameFrameTime;
	float systemFrameTime;
};

// --------------------------------------------------------------------
struct MatrixBufferData
{
	Matrix4 model;
	Matrix4 view;
	Matrix4 projection;
};

// --------------------------------------------------------------------
struct DirectionalLight
{
	Vector4 direction;
	Vector4 color;
};

struct PointLight
{
	Vector4 position;
	Vector4 color;
	Vector4 attenuation;
	Vector4 specAttenuation;
};

struct SpecularSurface
{
	float power;
	float factor;
	float padding[2];
};

struct LightBufferData
{
	Vector4 cameraEyePosition;
	
	Vector4 ambient;
	DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpecularSurface specularSurface;
};

struct MidiData
{
	float knobs[9];
	float sliders[9];
	float _padding[2];
};


// --------------------------------------------------------------------
class SimpleRenderer
{
public:
	RHIDevice*			m_device;
	RHIDeviceContext*	m_deviceContext;
	RHIOutput*			m_output;

    RHITextureBase*     m_currentTarget;

	Mesh*				m_global_mesh;
	VertexBuffer*		m_global_vbo;
	IndexBuffer*		m_global_ibo;

	Shader*				m_current_shader;
	const Material*	    m_current_material;

	RHITextureBase*		m_defaultDepthStencil;
	RHITextureBase*		m_currentDepthStencil;

	DepthStencilDesc	m_currentDepthDesc;
	DepthStencilState*	m_currentDepthStencilState;

	BlendState*			m_currentBlendState;
	BlendStateDesc		m_currentBlendStateDesc;

	Sampler*			m_linearSampler;
	Sampler*			m_pointSampler;

	ConstantBuffer*		m_matrixBuffer;
	ConstantBuffer*		m_timeBuffer;
	ConstantBuffer*		m_lightBuffer;
	ConstantBuffer*		m_midiBuffer;

	TimeBufferData		m_timeBufferData;
	MatrixBufferData	m_matrixBufferData;
	LightBufferData		m_lightBufferData;
	MidiData			m_midiData;

    Scene*              m_current_scene;

	static RasterState*			DEFAULT_RASTER_STATE;
	static BlendState*			DEFAULT_BLEND_STATE;
	static DepthStencilState*	DEFAULT_DEPTH_STENCIL_STATE;
	static RHITexture2D*		WHITE_TEXTURE;

public:
	RasterState*		m_solid_rasterstate;
	RasterState*		m_wireframe_rasterstate;

	RHITexture2D*		m_defaultTexture;
	ShaderProgram*		m_defaultShader;

public:
	RasterState*		m_noCullRasterState;
	ShaderProgram*		m_skyboxShader;
	CubeMap*			m_currentReflectionMap;

public:
	SimpleRenderer();
	~SimpleRenderer();

	// ---------------------------------
	// Lifecycle Methods
	// ---------------------------------
	// void Setup( RHIOutput *output );
	void Setup(unsigned int width, unsigned int height);
	// void Setup( Window *window );
	// void Setup( Texture2D *default_render_target );

	void Update(float deltaSeconds);

	void Destroy();



	// ---------------------------------
	// Display Functionality
	// ---------------------------------
	void SetDisplaySize(int width, int height);
	void SetDisplayMode(const RHIOutputMode rhiDisplayMode);



	// ---------------------------------
	// RenderTargets, Clear
	// ---------------------------------
	void SetColorTarget(RHITextureBase* colorTarget = nullptr, RHITextureBase* depthStencilTarget = nullptr, bool use_default_depth_target = true);
	void SetColorTargets(RHITextureBase** colorTargets, int numColorTargets, RHITextureBase* depthStencilTarget);
	void ClearColor(const Rgba& color) const;
	void ClearTargetColor(RHITextureBase* target, const Rgba& color) const;
	void ClearDepth(float depth = 1.0f, uint8_t stencil = 0U);



	// ---------------------------------
	// Movel, View, Projection Matrixes
	// ---------------------------------
	void set_model(const Transform& transform);
    void set_view(const Transform& transform);

	void SetModel(const Matrix4& model);
	void SetView(const Matrix4& view);

	void SetOrthoProjection(float minX, float maxX, float minY, float maxY, float minZ = 0.0f, float maxZ = 1.0f);
	void SetOrthoProjection(const Vector2& min, const Vector2& max);
	void SetOrthoProjection(const Vector3& min, const Vector3& max);
	void SetOrthoProjection(const AABB2& bounds);
	void SetAspectNormalizedOrtho();
	void SetPerspectiveProjection(float nz, float fz, float viewingAngleDegrees, float aspect);
	void SetProjection(const Matrix4& projection);



	// ---------------------------------
	// Lighting
	// ---------------------------------
	void SetEyePosition(const Vector3& eye);
	void SetAmbient(float intensity = 1.0f, const Rgba& color = Rgba::WHITE);

	void SetDirectionalLight(unsigned int index, const Vector3& direction, float intensity, const Rgba& color);
	void DisableDirectionLight(unsigned int index);

	void SetPointLight(unsigned int index, const Vector3& position, const Rgba& color, float intensity, const Vector3& attenuation, const Vector3& specAttenuation);
	void DisablePointLight(unsigned int index);

	void SetSpecularSurface(float specPower, float specFactor);
	void DisableSpecularSurface();



	// ---------------------------------
	// Blend State, Depth State, Raster State
	// ---------------------------------
	void set_blend_state(BlendState* blend_state) const;
	void set_depth_stencil_state(DepthStencilState* depth_state) const;
	void set_raster_state(RasterState* raster_state) const;

	void EnableWireframe();
	void DisableWireframe();

	void EnableBlend(BlendFactor srcFactor, BlendFactor dstFactor);
	void DisableBlend();

	void EnableDepth(bool enableTest, bool enableWrite, DepthTest depthTest = DEPTH_TEST_COMPARISON_LESS);
	void EnableDepthWrite(bool enabled);
	void EnableDepthTest(bool enabled);
	void SetDepthTest(DepthTest depthTest);



	// ---------------------------------
	// Viewport
	// ---------------------------------
	void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	void SetViewportAsPercent(float x, float y, float width, float height);



	// ---------------------------------
	// Shaders, Textures, Samplers
	// ---------------------------------
	Shader*		CreateShader(const char* shader_program_filepath);
	void		set_material(const Material* mat);

	void SetShader(Shader* shader) const;
	void SetShaderProgram(ShaderProgram* shader_program);

	void		SetTexture(unsigned int textureIndex, RHITextureBase* texture);
	inline void SetTexture(RHITextureBase* texture) { SetTexture(0, texture); }

	void		SetSampler(unsigned int samplerIndex, Sampler* sampler);
	inline void SetSampler(Sampler* sampler) { SetSampler(0, sampler); }



	// ---------------------------------
	// Compute 
	// ---------------------------------
    ComputeJob* create_compute_job(const char* compute_program_filepath);
    void        dispatch_job(ComputeJob* compute_job);



	// ---------------------------------
	// Constant Buffers
	// ---------------------------------
	void		SetConstantBuffer(unsigned int constantBufferIndex, ConstantBuffer* constantBuffer);
	inline void	SetConstantBuffer(ConstantBuffer* constantBuffer) {SetConstantBuffer(0, constantBuffer); };
	void		UpdateConstantBuffer(ConstantBuffer* constantBuffer, const void* data);

	void		SetStructuredBuffer(unsigned int structuredBufferIndex, StructuredBuffer* structuredBuffer);
	inline void	SetStructuredBuffer(StructuredBuffer* structuredBuffer) {SetStructuredBuffer(0, structuredBuffer); };
	void		UpdateStructuredBuffer(StructuredBuffer* constantBuffer, const void* data);


	// ---------------------------------
	// General Drawing
	// ---------------------------------
	void DrawVertexes(PrimitiveType topology, const Vertex3* vertexes, const unsigned int numVertexes);
	void DrawVertexesIndexed(PrimitiveType topology, const Vertex3* vertexes, const unsigned int numVertexes, const unsigned int* indexes, const unsigned int numIndexes);

	void DrawVBO(PrimitiveType topology, VertexBuffer *vbo);
	void DrawVBO(PrimitiveType topology, VertexBuffer *vbo, const unsigned int vertexCount, const unsigned int startIndex = 0);

	void DrawVBOIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo);
	void DrawVBOIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo, const unsigned int indexCount, const unsigned int startIndex = 0);

	void draw_with_meshbuilder(MeshBuilder& mb);
	void draw_mesh(Mesh* mesh);

	void Present() const;



	// ---------------------------------
	// 2d Drawing
	// ---------------------------------
	void DrawQuad2d(float minX, float minY, float maxX, float maxY, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	void DrawQuad2d(const Vector2& mins, const Vector2& maxs, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	void DrawQuad2d(const AABB2& bounds, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);

	void DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const char* text, const Font& font);
	void DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const std::vector<char>& text, const Font& font);
	void DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const std::string& text, const Font& font);
	void DrawText2dCentered(const Vector2& centeredPosition, float scale, const Rgba& tint, const std::string& text, const Font& font);
	void DrawText2dCenteredAndInBounds(const AABB2& bounds, const Rgba& tint, const std::string& text, const Font& font);


	
	// ---------------------------------
	// 3d World Drawing
	// ---------------------------------
	void DrawWorldAxes(float axisLength = 100.f);

	void DrawWorldGridXZ(float gridSize = 200.f, 
						 float majorUnitLength = 1.0f, 
						 float minorUnitLength = 0.2f, 
						 const Rgba& majorUnitColor = Rgba(255, 255, 255, 80), 
						 const Rgba& minorUnitColor = Rgba(255, 255, 255, 8));

	CubeMap*	LoadCubeMap(CubeMapImagePaths imagePaths);
	void		SetCurrentReflectionMap(CubeMap* cubeMap);
	void		BindCurrentReflectionMap(unsigned int index);
	void		DrawSkyBox(CubeMap* cubeMap);



	// ---------------------------------
	// 3d Shapes Drawing
	// ---------------------------------
	void DrawQuad3d(const Vector3& center,
					const Vector3& right,
					const Vector3& up,
					float widthHalfExtents,
					float heightHalfExtents,
					const AABB2& texCoords = AABB2::ZERO_TO_ONE,
					const Rgba& tint = Rgba::WHITE);

	void DrawTwoSidedQuad3d(const Vector3& center,
							const Vector3& right,
							const Vector3& up,
							float widthHalfExtents,
							float heightHalfExtents,
							const AABB2& texCoords = AABB2::ZERO_TO_ONE,
							const Rgba& tint = Rgba::WHITE);

	void DrawCube3d(const Vector3& center, float halfSize, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	void DrawInvertedCube3d(const Vector3& center, float halfSize, const AABB2& texCoords = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);

	void DrawUVSphere(const Vector3& center, float radius, const Rgba& color = Rgba::WHITE, unsigned int slices = 64);
	//void DrawIcoSphere();

	// ---------------------------------
	// 3d Debug Drawing
	// ---------------------------------

	void DebugDrawCross3d(const Vector3& position, const Rgba& color, float size);
	void DebugDrawDirectionalLights();
	void DebugDrawPointLights();

	void DebugDrawLine2d(const Vector2& start_pos, const Vector2& end_pos, float line_thickness = 1.0f, const Rgba start_color = Rgba::PINK, const Rgba end_color = Rgba::PINK);
	void DebugDrawLine3d(const Vector3& start_pos, const Vector3& end_pos, float line_thickness = 1.0f, const Rgba start_color = Rgba::PINK, const Rgba end_color = Rgba::PINK);

	void DebugDrawBox2d(const AABB2& bounds, float margin, float padding, const Rgba& edge_color, const Rgba& fill_color);

	void DebugDrawCircle2d(const Vector2& center, float radius, float margin, float padding, const Rgba& border_color, const Rgba& fill_color, int num_sides = 64);

	void draw_skeleton(const Skeleton* skeleton);
	void draw_skeleton_instance(const SkeletonInstance* skeleton_instance);

    Scene* get_current_scene();

private:
	void RecreateDefaultDepthBuffer();
	void update_midi_cb();
};