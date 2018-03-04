#include "Engine/Renderer/SimpleRenderer.hpp"
#include "Engine/RHI/RHI.hpp"
#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/Shader.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/StructuredBuffer.hpp"
#include "Engine/RHI/InlineShaders.hpp"
#include "Engine/RHI/ComputeJob.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Vertex3.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Meshes.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Input/midi.h"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Noise.hpp"

#include "Engine/Profile/profiler.h"

RasterState*		SimpleRenderer::DEFAULT_RASTER_STATE = nullptr;
BlendState*			SimpleRenderer::DEFAULT_BLEND_STATE = nullptr;
DepthStencilState*	SimpleRenderer::DEFAULT_DEPTH_STENCIL_STATE = nullptr;
RHITexture2D*		SimpleRenderer::WHITE_TEXTURE = nullptr;

SimpleRenderer::SimpleRenderer()
	:m_device(nullptr)
	,m_deviceContext(nullptr)
	,m_output(nullptr)
	,m_currentTarget(nullptr)
	,m_defaultDepthStencil(nullptr)
	,m_currentDepthStencil(nullptr)
	,m_currentDepthStencilState(nullptr)
	,m_solid_rasterstate(nullptr)
	,m_wireframe_rasterstate(nullptr)
	,m_noCullRasterState(nullptr)
	,m_currentBlendState(nullptr)
	,m_global_mesh(nullptr)
	,m_global_vbo(nullptr)
	,m_global_ibo(nullptr)
	,m_linearSampler(nullptr)
	,m_pointSampler(nullptr)
	,m_defaultTexture(nullptr)
	,m_defaultShader(nullptr)
	,m_timeBuffer(nullptr)
	,m_matrixBuffer(nullptr)
	,m_lightBuffer(nullptr)
	,m_midiBuffer(nullptr)
	,m_skyboxShader(nullptr)
	,m_currentReflectionMap(nullptr)
    ,m_current_scene(nullptr)
{
	MemZero(&m_timeBufferData);
	MemZero(&m_lightBufferData);
	MemZero(&m_currentBlendStateDesc);
	MemZero(&m_midiData);
}

SimpleRenderer::~SimpleRenderer()
{
	Destroy();
}

void SimpleRenderer::Setup(unsigned int width, unsigned int height)
{
	// Create RHI objects
	RHIInstance& rhiInstance = RHIInstance::GetInstance();
	rhiInstance.CreateOuput(&m_deviceContext, &m_output, width, height, RHI_OUTPUT_MODE_WINDOWED);
	m_device = m_deviceContext->m_device;


	// Built in buffers 
	m_global_mesh = new Mesh();
	m_global_vbo = m_device->CreateVertexBuffer(nullptr, 1, BUFFERUSAGE_DYNAMIC);
	m_global_ibo = m_device->CreateIndexBuffer(nullptr, 1, BUFFERUSAGE_DYNAMIC);

	//----------------------
	// DEFAULTS

	// Raster state
	DEFAULT_RASTER_STATE = new RasterState(m_device);

	// Blend state
	BlendStateDesc blend_state_desc = BlendStateDesc(true, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	DEFAULT_BLEND_STATE = new BlendState(m_device, blend_state_desc);

	// Depth stencil state
	DepthStencilDesc default_depth_stencil_desc;
	DEFAULT_DEPTH_STENCIL_STATE = new DepthStencilState(m_device, default_depth_stencil_desc);

	WHITE_TEXTURE = new RHITexture2D(m_device, Rgba::WHITE);
	//----------------------

	// Built in samplers
	SamplerDescription desc;
	desc.SetFilter(FILTER_MODE_LINEAR);
	m_linearSampler = m_device->CreateSampler(desc);
	SetSampler(0, m_linearSampler);

	desc.SetFilter(FILTER_MODE_POINT);
	m_pointSampler = m_device->CreateSampler(desc);
	SetSampler(1, m_pointSampler);

	// Setup defaults
	m_solid_rasterstate = new RasterState(m_device, CULL_BACK, FILL_SOLID);
	m_wireframe_rasterstate = new RasterState(m_device, CULL_BACK, FILL_WIREFRAME);

	m_deviceContext->SetRasterState(m_solid_rasterstate);

	m_noCullRasterState = new RasterState(m_device, CULL_NONE);

	m_defaultTexture = m_device->CreateRHITexture2DFromColor(Rgba::WHITE);
	SetTexture(0, m_defaultTexture);

	m_defaultShader = m_device->CreateEmptyShader();
	m_defaultShader->load_vertex_stage_from_text(DEFAULT_VERTEX_SHADER_TEXT);
	m_defaultShader->load_fragment_stage_from_text(DEFAULT_FRAGMENT_SHADER_TEXT);
	SetShaderProgram(m_defaultShader);

	m_skyboxShader = m_device->CreateShaderFromText(SKYBOX_SHADER_RAW_TEXT);

	// Setup mvp matrices
	m_matrixBuffer = m_device->CreateConstantBuffer(&m_matrixBufferData, sizeof(m_matrixBufferData));
	SetConstantBuffer(MATRIX_BUFFER_INDEX, m_matrixBuffer);

	m_timeBuffer = m_device->CreateConstantBuffer(&m_timeBufferData, sizeof(m_timeBufferData));
	SetConstantBuffer(TIME_BUFFER_INDEX, m_timeBuffer);

	m_lightBuffer = m_device->CreateConstantBuffer(&m_lightBufferData, sizeof(m_lightBufferData));
	SetConstantBuffer(LIGHT_BUFFER_INDEX, m_lightBuffer);

	for(u8 i = 0; i < 9; i++){
		midi_bindf_knob(i, &m_midiData.knobs[i], 0.0f, 1.0f);
		midi_bindf_slider(i, &m_midiData.sliders[i], 0.0f, 1.0f);
	}

	m_midiBuffer = m_device->CreateConstantBuffer(&m_midiData, sizeof(m_midiData));
	SetConstantBuffer(MIDI_BUFFER_INDEX, m_midiBuffer);

    Material::init();

	RecreateDefaultDepthBuffer();

	EnableDepth(true, true);

	SetColorTarget(nullptr, nullptr);

    m_current_scene = new Scene();
}

void SimpleRenderer::Update(float deltaSeconds)
{
    PROFILE_SCOPE_FUNCTION();
	m_timeBufferData.gameTime += deltaSeconds;
	m_timeBufferData.systemTime += deltaSeconds;
	m_timeBufferData.gameFrameTime = deltaSeconds;
	m_timeBufferData.systemFrameTime = deltaSeconds;

	m_timeBuffer->Update(m_deviceContext, &m_timeBufferData);

	update_midi_cb();
}

void SimpleRenderer::Destroy() 
{
    SAFE_DELETE(m_current_scene);

	SAFE_DELETE(m_solid_rasterstate);
	SAFE_DELETE(m_wireframe_rasterstate);
	SAFE_DELETE(m_noCullRasterState);
	SAFE_DELETE(m_defaultTexture);
	SAFE_DELETE(m_defaultShader);
	SAFE_DELETE(m_skyboxShader)

	SAFE_DELETE(m_matrixBuffer);
	SAFE_DELETE(m_timeBuffer);
    SAFE_DELETE(m_lightBuffer);
    SAFE_DELETE(m_midiBuffer);

	SAFE_DELETE(m_global_mesh);
	SAFE_DELETE(m_global_vbo);
	SAFE_DELETE(m_global_ibo);

	SAFE_DELETE(m_linearSampler);
	SAFE_DELETE(m_pointSampler);

	SAFE_DELETE(m_currentBlendState);
    SAFE_DELETE(m_currentDepthStencilState);
    SAFE_DELETE(m_currentDepthStencil);

	SAFE_DELETE(DEFAULT_BLEND_STATE);
	SAFE_DELETE(DEFAULT_DEPTH_STENCIL_STATE);
	SAFE_DELETE(DEFAULT_RASTER_STATE);
	SAFE_DELETE(WHITE_TEXTURE);

	SAFE_DELETE(m_output);
	SAFE_DELETE(m_deviceContext);
	SAFE_DELETE(m_device);
};

void SimpleRenderer::SetDisplaySize(int width, int height)
{
	m_output->SetDisplaySize(width, height);
	RecreateDefaultDepthBuffer();
}

void SimpleRenderer::SetDisplayMode(const RHIOutputMode rhiDisplayMode)
{
	m_output->SetDisplayMode(rhiDisplayMode);
	RecreateDefaultDepthBuffer();
}

void SimpleRenderer::SetColorTarget(RHITextureBase* colorTarget, RHITextureBase* depthStencilTarget, bool use_default_depth_target) 
{ 
	if(colorTarget){
		m_currentTarget = colorTarget;
	} else{
		m_currentTarget = m_output->m_renderTarget;
	}

	if(depthStencilTarget == nullptr && use_default_depth_target){
		m_currentDepthStencil = m_defaultDepthStencil;
	}
	else{
		m_currentDepthStencil = depthStencilTarget;
	}

	m_deviceContext->SetColorTarget(m_currentTarget, m_currentDepthStencil);
};

void SimpleRenderer::SetColorTargets(RHITextureBase** colorTargets, int numColorTargets, RHITextureBase* depthStencilTarget)
{
	m_deviceContext->SetColorTargets(colorTargets, numColorTargets, depthStencilTarget);

	if(depthStencilTarget == nullptr){
		m_currentDepthStencil = m_defaultDepthStencil;
	}
	else{
		m_currentDepthStencil = depthStencilTarget;
	}
}

void SimpleRenderer::ClearColor(const Rgba &color) const
{
    PROFILE_SCOPE_FUNCTION();
	ClearTargetColor(m_currentTarget, color);
};

void SimpleRenderer::ClearTargetColor(RHITextureBase *target, const Rgba& color) const
{
	m_deviceContext->ClearColorTarget(target, color);
}

void SimpleRenderer::ClearDepth(float depth, uint8_t stencil)
{
	m_deviceContext->ClearDepthTarget(m_currentDepthStencil, depth, stencil);
}

void SimpleRenderer::set_model(const Transform& transform)
{
	m_matrixBufferData.model = transform.calc_world_matrix();
	m_matrixBuffer->Update(m_deviceContext, &m_matrixBufferData);
}

void SimpleRenderer::set_view(const Transform& transform)
{
	m_matrixBufferData.view = transform.calc_world_matrix();
	m_matrixBuffer->Update(m_deviceContext, &m_matrixBufferData);
}

void SimpleRenderer::SetModel(const Matrix4& model)
{
    PROFILE_SCOPE_FUNCTION();
	m_matrixBufferData.model = model;
	m_matrixBuffer->Update(m_deviceContext, &m_matrixBufferData);
}

void SimpleRenderer::SetView(const Matrix4& view)
{
    PROFILE_SCOPE_FUNCTION();
	m_matrixBufferData.view = view;
	m_matrixBuffer->Update(m_deviceContext, &m_matrixBufferData);
}

void SimpleRenderer::SetOrthoProjection(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
    PROFILE_SCOPE_FUNCTION();
	Matrix4 projection = RHIInstance::GetInstance().CreateOrthoProjection(minX, maxX, minY, maxY, minZ, maxZ);
	SetProjection(projection);
}

void SimpleRenderer::SetOrthoProjection(const Vector2& min, const Vector2& max)
{
	SetOrthoProjection(min.x, max.x, min.y, max.y, 0.0f, 1.0f);
}

void SimpleRenderer::SetOrthoProjection(const Vector3& min, const Vector3& max)
{
	SetOrthoProjection(min.x, max.x, min.y, max.y, min.z, max.z);
}

void SimpleRenderer::SetOrthoProjection(const AABB2& bounds)
{
	SetOrthoProjection(bounds.mins, bounds.maxs);
}

void SimpleRenderer::SetAspectNormalizedOrtho()
{
	float aspect = m_output->GetAspectRatio();
	Vector2 mins(0.0f, 0.0f);
	Vector2 maxs(aspect, 1.0f);
	SetOrthoProjection(mins, maxs);
}

void SimpleRenderer::SetPerspectiveProjection(float nz, float fz, float viewingAngleDegrees, float aspect)
{
	Matrix4 perspective = RHIInstance::GetInstance().CreatePerspectiveProjection(nz, fz, viewingAngleDegrees, aspect);
	SetProjection(perspective);
}

void SimpleRenderer::SetProjection(const Matrix4& projection)
{
	m_matrixBufferData.projection = projection;
	m_matrixBuffer->Update(m_deviceContext, &m_matrixBufferData);
}

void SimpleRenderer::SetEyePosition(const Vector3& eye)
{
	m_lightBufferData.cameraEyePosition.xyz = eye;
	m_lightBuffer->Update(m_deviceContext, &m_lightBufferData);
}

void SimpleRenderer::SetAmbient(float intensity, const Rgba& color)
{
	color.GetAsFloats(&m_lightBufferData.ambient.x);
	m_lightBufferData.ambient.w = intensity;
	m_lightBuffer->Update(m_deviceContext, &m_lightBufferData);
}

void SimpleRenderer::SetDirectionalLight(unsigned int index, const Vector3& direction, float intensity, const Rgba& color)
{
	ASSERT_OR_DIE(index < MAX_DIRECTIONAL_LIGHTS, Stringf("Trying to enable invalid directional light index. Max index is %u\n", MAX_DIRECTIONAL_LIGHTS - 1));

	// Set the direction
	m_lightBufferData.directionalLights[index].direction.xyz = direction;
	
	// Set the color & intensity
	color.GetAsFloats(&m_lightBufferData.directionalLights[index].color.x);
	m_lightBufferData.directionalLights[index].color.w = intensity;

	m_lightBuffer->Update(m_deviceContext, &m_lightBufferData);
}

void SimpleRenderer::DisableDirectionLight(unsigned int index)
{
	ASSERT_OR_DIE(index < MAX_DIRECTIONAL_LIGHTS, Stringf("Trying to disable invalid directional light index. Max index is %u\n", MAX_DIRECTIONAL_LIGHTS - 1));

	m_lightBufferData.directionalLights[index].direction = Vector4::ZERO;
	m_lightBufferData.directionalLights[index].color = Vector4::ZERO;

	m_lightBuffer->Update(m_deviceContext, &m_lightBufferData);
}

void SimpleRenderer::SetPointLight(unsigned int index, const Vector3& position, const Rgba& color, float intensity, const Vector3& attenuation, const Vector3& specAttenuation)
{
	ASSERT_OR_DIE(index < MAX_POINT_LIGHTS, Stringf("Trying to enable invalid point light index. Max index is %u\n", MAX_POINT_LIGHTS - 1));

	// Set the position
	m_lightBufferData.pointLights[index].position = Vector4(position, 1.0f);

	// Set the colors as floats
	color.GetAsFloats(&m_lightBufferData.pointLights[index].color.x);
	m_lightBufferData.pointLights[index].color.w = intensity;

	// Set the attunuation
	m_lightBufferData.pointLights[index].attenuation.xyz = attenuation;

	// Set the spec attenuation
	m_lightBufferData.pointLights[index].specAttenuation.xyz = specAttenuation;

	// Update the buffer
	m_lightBuffer->Update(m_deviceContext, &m_lightBufferData);
}

void SimpleRenderer::DisablePointLight(unsigned int index)
{
	ASSERT_OR_DIE(index < MAX_POINT_LIGHTS, Stringf("Trying to disable invalid point light index. Max index is %u\n", MAX_POINT_LIGHTS - 1));

	m_lightBufferData.pointLights[index].position = Vector4::ZERO;
	m_lightBufferData.pointLights[index].color = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	m_lightBufferData.pointLights[index].attenuation = Vector4(1.0f, 0.0f, 0.0f, 0.0f);

	m_lightBuffer->Update(m_deviceContext, &m_lightBufferData);
}

void SimpleRenderer::SetSpecularSurface(float specPower, float specFactor)
{
	m_lightBufferData.specularSurface.power = specPower;
	m_lightBufferData.specularSurface.factor = specFactor;

	m_lightBuffer->Update(m_deviceContext, &m_lightBufferData);
}

void SimpleRenderer::DisableSpecularSurface()
{
	SetSpecularSurface(0.0f, 0.0f);
}

void SimpleRenderer::set_blend_state(BlendState* blend_state) const
{
    PROFILE_SCOPE_FUNCTION();
	m_deviceContext->SetBlendState(blend_state);
}

void SimpleRenderer::set_depth_stencil_state(DepthStencilState* depth_state) const
{
    PROFILE_SCOPE_FUNCTION();
	m_deviceContext->SetDepthStencilState(depth_state);
}

void SimpleRenderer::set_raster_state(RasterState* raster_state) const
{
    PROFILE_SCOPE_FUNCTION();
	m_deviceContext->SetRasterState(raster_state);
}

void SimpleRenderer::EnableWireframe()
{
	set_raster_state(m_wireframe_rasterstate);
}

void SimpleRenderer::DisableWireframe()
{
	set_raster_state(m_solid_rasterstate);
}

void SimpleRenderer::EnableBlend(BlendFactor srcFactor, BlendFactor dstFactor)
{
    PROFILE_SCOPE_FUNCTION();
	SAFE_DELETE(m_currentBlendState);

	BlendStateDesc blend_state_desc(true, srcFactor, dstFactor);
	BlendState* blendState = new BlendState(m_device, blend_state_desc);
	m_deviceContext->SetBlendState(blendState);

	// Save off the flags to a small struct
	m_currentBlendStateDesc = blend_state_desc;
	m_currentBlendState = blendState;
}

void SimpleRenderer::DisableBlend()
{
    PROFILE_SCOPE_FUNCTION();
	if (!m_currentBlendStateDesc.m_enabled){
		return;
	}

	SAFE_DELETE(m_currentBlendState);

	m_currentBlendStateDesc.m_enabled = false;

	BlendState* blendState = new BlendState(m_device, m_currentBlendStateDesc);
	m_deviceContext->SetBlendState(m_currentBlendState);

	m_currentBlendState = blendState;
}

void SimpleRenderer::EnableDepth(bool enableTest, bool enableWrite, DepthTest depthTest)
{
    PROFILE_SCOPE_FUNCTION();
	SAFE_DELETE(m_currentDepthStencilState);

	m_currentDepthDesc.m_depthTestEnabled = enableTest;
	m_currentDepthDesc.m_depthWritingEnabled = enableWrite;
	m_currentDepthDesc.m_depthTest = depthTest;

	DepthStencilState* depthStencilState = new DepthStencilState(m_device, m_currentDepthDesc);

	m_deviceContext->SetDepthStencilState(depthStencilState);

	m_currentDepthStencilState = depthStencilState;
}

void SimpleRenderer::EnableDepthWrite(bool enabled)
{
	if(m_currentDepthDesc.m_depthWritingEnabled != enabled){
		EnableDepth(m_currentDepthDesc.m_depthTestEnabled, enabled, m_currentDepthDesc.m_depthTest);
	}
}

void SimpleRenderer::EnableDepthTest(bool enabled)
{
	if(m_currentDepthDesc.m_depthTestEnabled != enabled){
		EnableDepth(enabled, m_currentDepthDesc.m_depthWritingEnabled, m_currentDepthDesc.m_depthTest);
	}
}

void SimpleRenderer::SetDepthTest(DepthTest depthTest)
{
	if(m_currentDepthDesc.m_depthTest != depthTest){
		EnableDepth(m_currentDepthDesc.m_depthTestEnabled, m_currentDepthDesc.m_depthWritingEnabled, m_currentDepthDesc.m_depthTest);
	}
}

void SimpleRenderer::SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	m_deviceContext->SetViewport(x, y, width, height);
}

void SimpleRenderer::SetViewportAsPercent(float xPercent, float yPercent, float widthPercent, float heightPercent)
{
	float targetWidth = (float)m_output->GetWidth();
	float targetHeight = (float)m_output->GetHeight();

	unsigned int x = (unsigned int)(targetWidth * xPercent);
	unsigned int y = (unsigned int)(targetHeight * yPercent);
	unsigned int width = (unsigned int)(targetWidth * widthPercent);
	unsigned int height = (unsigned int)(targetHeight * heightPercent);

	m_deviceContext->SetViewport(x, y, width, height);
}

void SimpleRenderer::Present() const
{
    PROFILE_SCOPE_FUNCTION();
	m_output->Present();
};

Shader* SimpleRenderer::CreateShader(const char* shader_program_filepath)
{
	ShaderProgram* shader_program = new ShaderProgram(m_device, shader_program_filepath);
	Shader* shader = new Shader(shader_program);

	return shader;
}

void SimpleRenderer::set_material(const Material* mat)
{
    PROFILE_SCOPE_FUNCTION();
	// Set Textures
	for(int texture_index = 0; texture_index < MAX_NUM_TEXTURES; ++texture_index){
		RHITexture2D* texture = mat->get_texture(texture_index);
		SetTexture(texture_index, texture);
	}

	// Set Constant Buffers
	for(int cb_index = 0; cb_index < MAX_NUM_CONSTANT_BUFFERS; ++cb_index){
		switch(cb_index){
			case MATRIX_BUFFER_INDEX:	SetConstantBuffer(MATRIX_BUFFER_INDEX, m_matrixBuffer); break;
			case TIME_BUFFER_INDEX:		SetConstantBuffer(TIME_BUFFER_INDEX, m_timeBuffer); break;
			case LIGHT_BUFFER_INDEX:	SetConstantBuffer(LIGHT_BUFFER_INDEX, m_lightBuffer); break;
			default:{
				ConstantBuffer* cb = mat->get_constant_buffer(cb_index);
				SetConstantBuffer(cb_index, cb);
			}break;
		}
	}

	// Set Raster State
	set_raster_state(mat->m_shader->get_raster_state());

	// Set Depth Stencil State
	set_depth_stencil_state(mat->m_shader->get_depth_stencil_state());

	// Set Blend State
	set_blend_state(mat->m_shader->get_blend_state());

	m_deviceContext->SetShaderProgram(mat->m_shader->m_shader_program);

	m_current_material = mat;
}

void SimpleRenderer::SetShader(Shader* shader) const
{
    PROFILE_SCOPE_FUNCTION();
	if(shader){
		m_deviceContext->SetShaderProgram(shader->m_shader_program);
		set_raster_state(shader->m_raster_state);
		set_depth_stencil_state(shader->m_depth_stencil_state);
		set_blend_state(shader->m_blend_state);
	}
	else{
		m_deviceContext->SetShaderProgram(m_defaultShader);
	}
}

void SimpleRenderer::SetShaderProgram(ShaderProgram* shader_program)
{
    PROFILE_SCOPE_FUNCTION();
	if(shader_program){
		m_deviceContext->SetShaderProgram(shader_program);
	}
	else{
		m_deviceContext->SetShaderProgram(m_defaultShader);
	}
}

void SimpleRenderer::SetTexture(unsigned int textureIndex, RHITextureBase *texture)
{
    PROFILE_SCOPE_FUNCTION();
	if (texture){
		m_deviceContext->SetTexture(textureIndex, texture);
	}
	else{
		m_deviceContext->SetTexture(textureIndex, m_defaultTexture);
	}
}

void SimpleRenderer::SetSampler(unsigned int sampler_index, Sampler *sampler)
{
	m_deviceContext->SetSampler(sampler_index, sampler);
}

ComputeJob* SimpleRenderer::create_compute_job(const char* compute_program_filepath)
{
    return new ComputeJob(m_device, compute_program_filepath);
}

void SimpleRenderer::dispatch_job(ComputeJob* compute_job)
{
    m_deviceContext->DispatchComputeJob(compute_job);
}

void SimpleRenderer::SetConstantBuffer(unsigned int constantBufferIndex, ConstantBuffer* constantBuffer)
{
	m_deviceContext->SetConstantBuffer(constantBufferIndex, constantBuffer);
}

void SimpleRenderer::UpdateConstantBuffer(ConstantBuffer* constantBuffer, const void* data)
{
	constantBuffer->Update(m_deviceContext, data);
}

void SimpleRenderer::SetStructuredBuffer(unsigned int structuredBufferIndex, StructuredBuffer* structuredBuffer)
{
	m_deviceContext->SetStructuredBuffer(structuredBufferIndex, structuredBuffer);
}

void SimpleRenderer::UpdateStructuredBuffer(StructuredBuffer* structuredBuffer, const void* data)
{
	structuredBuffer->update(m_deviceContext, data);
}

void SimpleRenderer::DrawVertexes(PrimitiveType topology, const Vertex3* vertexes, const unsigned int numVertexes)
{
	m_global_vbo->Update(m_deviceContext, vertexes, numVertexes);
	DrawVBO(topology, m_global_vbo, numVertexes);
}

void SimpleRenderer::DrawVertexesIndexed(PrimitiveType topology, const Vertex3* vertexes, const unsigned int numVertexes, const unsigned int* indexes, const unsigned int numIndexes)
{
	m_global_vbo->Update(m_deviceContext, vertexes, numVertexes);
	m_global_ibo->Update(m_deviceContext, indexes, numIndexes);
	DrawVBOIndexed(topology, m_global_vbo, m_global_ibo);
}

void SimpleRenderer::DrawVBO(PrimitiveType topology, VertexBuffer *vbo)
{
	DrawVBO(topology, vbo, vbo->m_numVertexes, 0);
}

void SimpleRenderer::DrawVBO(PrimitiveType topology, VertexBuffer *vbo, const unsigned int vertexCount, const unsigned int startIndex) 
{
	ASSERT_OR_DIE(vbo, "VBO is nullptr");
	m_deviceContext->Draw(topology, vbo, vertexCount, startIndex);
}

void SimpleRenderer::DrawVBOIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo)
{
	DrawVBOIndexed(topology, vbo, ibo, ibo->m_numIndexes, 0);
}

void SimpleRenderer::DrawVBOIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo, const unsigned int indexCount, const unsigned int startIndex)
{
	ASSERT_OR_DIE(vbo, "VBO is nullptr");
	ASSERT_OR_DIE(ibo, "IBO is nullptr");
	m_deviceContext->DrawIndexed(topology, vbo, ibo, indexCount, startIndex);
}

void SimpleRenderer::draw_with_meshbuilder(MeshBuilder& mb)
{
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::draw_mesh(Mesh* mesh)
{
	for(const draw_instruction_t& t : mesh->m_draw_instructions){
		if(t.uses_index_buffer){
			DrawVBOIndexed(t.primitive_type, mesh->m_vbo, mesh->m_ibo, t.count, t.start_index);
		}
		else{
			DrawVBO(t.primitive_type, mesh->m_vbo, t.count, t.start_index);
		}
	}
}

void SimpleRenderer::DrawQuad2d(float minX, float minY, float maxX, float maxY, const AABB2& texCoords, const Rgba& tint)
{
	MeshBuilder mb;
	Meshes::build_quad_2d(mb, minX, minY, maxX, maxY, texCoords, tint, tint);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawQuad2d(const Vector2& mins, const Vector2& maxs, const AABB2& texCoords, const Rgba& tint)
{
	MeshBuilder mb;
	Meshes::build_quad_2d(mb, mins, maxs, texCoords, tint, tint);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawQuad2d(const AABB2& bounds, const AABB2& texCoords, const Rgba& tint)
{
	DrawQuad2d(bounds.mins, bounds.maxs, texCoords, tint);
}

void SimpleRenderer::DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const char* text, const Font& font)
{
    PROFILE_SCOPE_FUNCTION();
	EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	SetTexture(font.m_fontTexture);
    SetShaderProgram(m_defaultShader);

	MeshBuilder mb;
	Meshes::build_text_2d(mb, topLeftPosition, scale, tint, text, font);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const std::vector<char>& text, const Font& font)
{
    PROFILE_SCOPE_FUNCTION();
	EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	SetTexture(font.m_fontTexture);

	MeshBuilder mb;
	Meshes::build_text_2d(mb, topLeftPosition, scale, tint, text, font);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawText2d(const Vector2& topLeftPosition, float scale, const Rgba& tint, const std::string& text, const Font& font)
{
    PROFILE_SCOPE_FUNCTION();
	EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	SetTexture(font.m_fontTexture);

	MeshBuilder mb;
	Meshes::build_text_2d(mb, topLeftPosition, scale, tint, text, font);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawText2dCentered(const Vector2& centeredPosition, float scale, const Rgba& tint, const std::string& text, const Font& font)
{
    PROFILE_SCOPE_FUNCTION();
	EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	SetTexture(font.m_fontTexture);

	MeshBuilder mb;
	Meshes::build_text_2d_centered(mb, centeredPosition, scale, tint, text, font);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawText2dCenteredAndInBounds(const AABB2& bounds, const Rgba& tint, const std::string& text, const Font& font)
{
    PROFILE_SCOPE_FUNCTION();
	EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	SetTexture(font.m_fontTexture);

	MeshBuilder mb;
	Meshes::build_text_2d_centered_and_in_bounds(mb, bounds, tint, text, font);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawWorldAxes(float axisLength)
{
    PROFILE_SCOPE_FUNCTION();
	SetTexture(nullptr);
	SetShaderProgram(m_defaultShader);

	//TODO: set_material(m_diffuse_unlit_material);

	MeshBuilder mb;
	Meshes::build_scene_axes(mb, axisLength);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawWorldGridXZ(float gridSize, 
									 float majorUnitLength, 
									 float minorUnitLength, 
									 const Rgba& majorUnitColor, 
									 const Rgba& minorUnitColor)
{
    PROFILE_SCOPE_FUNCTION();
	SetTexture(nullptr);
	SetShaderProgram(m_defaultShader);

	//TODO: set_material(m_diffuse_unlit_material);

	MeshBuilder mb;
	Meshes::build_scene_grid_xz(mb, gridSize, majorUnitLength, minorUnitLength, majorUnitColor, minorUnitColor);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

CubeMap* SimpleRenderer::LoadCubeMap(CubeMapImagePaths imagePaths)
{
	return new CubeMap(m_device, imagePaths);
}

void SimpleRenderer::SetCurrentReflectionMap(CubeMap* cubeMap)
{
	m_currentReflectionMap = cubeMap;
}

void SimpleRenderer::BindCurrentReflectionMap(unsigned int index)
{
	SetTexture(index, m_currentReflectionMap->m_texture);
}

void SimpleRenderer::DrawSkyBox(CubeMap* cubeMap)
{
	SetTexture(0, cubeMap->m_texture);
	SetShaderProgram(m_skyboxShader);
	m_deviceContext->SetRasterState(m_noCullRasterState);
	EnableDepth(true, false, DEPTH_TEST_COMPARISON_LESS_EQUAL);

	DrawUVSphere(Vector3::ZERO, 1.0f);

	EnableDepth(true, true, DEPTH_TEST_COMPARISON_LESS);
	m_deviceContext->SetRasterState(m_solid_rasterstate);
	SetShader(nullptr);
}

void SimpleRenderer::DrawQuad3d(const Vector3& center, 
								const Vector3& right, 
								const Vector3& up, 
								float widthHalfExtents, 
								float heightHalfExtents, 
								const AABB2& texCoords, 
								const Rgba& tint)
{
	MeshBuilder mb;
	Meshes::build_quad_3d(mb, center, right, up, widthHalfExtents, heightHalfExtents, texCoords, tint);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawTwoSidedQuad3d(const Vector3& center,
										const Vector3& right,
										const Vector3& up,
										float widthHalfExtents,
										float heightHalfExtents,
										const AABB2& texCoords,
										const Rgba& tint)
{
	MeshBuilder mb;
	Meshes::build_two_sided_quad_3d(mb, center, right, up, widthHalfExtents, heightHalfExtents, texCoords, tint);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawCube3d(const Vector3& center, float halfSize, const AABB2& texCoords, const Rgba& tint)
{
	MeshBuilder mb;
	Meshes::build_cube_3d(mb, center, halfSize, texCoords, tint);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawInvertedCube3d(const Vector3& center, float halfSize, const AABB2& texCoords, const Rgba& tint)
{
	MeshBuilder mb;
	Meshes::build_inverted_cube_3d(mb, center, halfSize, texCoords, tint);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DrawUVSphere(const Vector3& center, float radius, const Rgba& color, unsigned int slices)
{
	MeshBuilder mb;
	Meshes::build_uv_sphere(mb, center, radius, color, slices);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DebugDrawCross3d(const Vector3& position, const Rgba& color, float size)
{
	UNUSED(position);
	UNUSED(color);
	UNUSED(size);
}

void SimpleRenderer::DebugDrawDirectionalLights()
{
	SetShader(nullptr);
	SetTexture(nullptr);

	for(int dirLightIndex = 0; dirLightIndex < MAX_DIRECTIONAL_LIGHTS; ++dirLightIndex){
		const DirectionalLight& dirLight = m_lightBufferData.directionalLights[dirLightIndex];

		DebugDrawCross3d(dirLight.direction.xyz * -5.0f, Rgba(dirLight.color), 0.25f );
	}
}

void SimpleRenderer::DebugDrawPointLights()
{
	SetShader(nullptr);
	SetTexture(nullptr);

	for(int pointLightIndex = 0; pointLightIndex < MAX_POINT_LIGHTS; ++pointLightIndex){
		const PointLight& light = m_lightBufferData.pointLights[pointLightIndex];
		DebugDrawCross3d(light.position.xyz, Rgba(light.color), 0.25f);
	}
}

void SimpleRenderer::DebugDrawLine2d(const Vector2& start_pos, const Vector2& end_pos, float line_thickness, const Rgba start_color, const Rgba end_color)
{
	SetShader(nullptr);
	SetTexture(nullptr);

	MeshBuilder mb;
	Meshes::build_line_2d(mb, start_pos, end_pos, line_thickness, start_color, end_color);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DebugDrawLine3d(const Vector3& start_pos, const Vector3& end_pos, float line_thickness, const Rgba start_color, const Rgba end_color)
{
	SetShader(nullptr);
	SetTexture(nullptr);

	MeshBuilder mb;
	Meshes::build_line_3d(mb, start_pos, end_pos, line_thickness, start_color, end_color);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DebugDrawBox2d(const AABB2& bounds, float margin, float padding, const Rgba& edge_color, const Rgba& fill_color)
{
	SetShader(nullptr);
	SetTexture(nullptr);

	MeshBuilder mb;
	Meshes::build_box_2d(mb, bounds, margin, padding, edge_color, fill_color);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::DebugDrawCircle2d(const Vector2& center, float radius, float margin, float padding, const Rgba& border_color, const Rgba& fill_color, int num_sides)
{
	SetShader(nullptr);
	SetTexture(nullptr);

	MeshBuilder mb;
	Meshes::build_circle_2d(mb, center, radius, margin, padding, border_color, fill_color, num_sides);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::draw_skeleton(const Skeleton* skeleton)
{
	SetShader(nullptr);
	SetTexture(nullptr);

	MeshBuilder mb;
	Meshes::build_skeleton(mb, skeleton);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

void SimpleRenderer::draw_skeleton_instance(const SkeletonInstance* skeleton_instance)
{
	SetShader(nullptr);
	SetTexture(nullptr);

	MeshBuilder mb;
	Meshes::build_skeleton_instance(mb, skeleton_instance);
	mb.copy_to_mesh(m_global_mesh);
	draw_mesh(m_global_mesh);
}

Scene* SimpleRenderer::get_current_scene()
{
    return m_current_scene;
}

void SimpleRenderer::RecreateDefaultDepthBuffer()
{
	SAFE_DELETE(m_defaultDepthStencil);
	m_defaultDepthStencil = new RHITexture2D(m_device, m_output->GetWidth(), m_output->GetHeight(), IMAGE_FORMAT_D24S8);
	m_currentDepthStencil = m_defaultDepthStencil;
}

void SimpleRenderer::update_midi_cb()
{
	m_midiBuffer->Update(m_deviceContext, &m_midiData);
}