#pragma once

#include "Engine/Engine.hpp"
#include "Engine/Core/xml.hpp"
#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/DX11.hpp"

class ShaderProgram;

class Shader
{
public:
	ShaderProgram*		m_shader_program;

	RasterState*		m_raster_state;
	DepthStencilState*	m_depth_stencil_state;
	BlendState*			m_blend_state;

public:
	static std::map<std::string, Shader*> s_database;
	static Shader* find_or_create(const char* shader_xml_file);

public:
	Shader(const char* shader_xml_file);
	Shader(ShaderProgram* shader_program, 
		   RasterState* rs = SimpleRenderer::DEFAULT_RASTER_STATE, 
		   DepthStencilState* dss = SimpleRenderer::DEFAULT_DEPTH_STENCIL_STATE, 
		   BlendState* bs = SimpleRenderer::DEFAULT_BLEND_STATE);
	~Shader();

	Shader* clone() const;

	void set_shader_program(ShaderProgram* shader_program);

	void set_raster_state(RasterState* raster_state);
	void set_depth_stencil_state(DepthStencilState* depth_stencil_state);
	void set_blend_state(BlendState* blend_state);

	RasterState*		get_raster_state() const;
	DepthStencilState*	get_depth_stencil_state() const;
	BlendState*			get_blend_state() const;

	int find_bind_index_for_name(const char* bind_name) const;

private:
	ShaderProgram* create_shader_program_from_xml(const XMLElement& shader_program_xml);
	RasterState* create_raster_state_from_xml(const XMLElement& raster_xml);
	DepthStencilState* create_depth_state_from_xml(const XMLElement& depth_xml);
	BlendState* create_blend_state_from_xml(const XMLElement& blend_xml);
};