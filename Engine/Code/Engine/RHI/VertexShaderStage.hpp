#pragma once

#include "Engine/RHI/ShaderStage.hpp"
#include "Engine/Config/EngineConfig.hpp"

class VertexShaderStage : public ShaderStage
{
public:
	ID3D11VertexShader* m_dx_vertex_shader;
	ID3D11InputLayout*	m_dx_input_layout;

public:
	VertexShaderStage(RHIDevice* rhi_device, 
					  const char* shader_raw_source, 
					  const size_t shader_raw_source_size, 
					  const char* entry_point = DEFAULT_VERTEX_SHADER_ENTRY_POINT);

	virtual ~VertexShaderStage() override;

	bool is_valid() const { return m_dx_vertex_shader != nullptr; }

private:
	void create_input_layout(RHIDevice* rhi_device);
};