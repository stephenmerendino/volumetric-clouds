#pragma once

#include "Engine/RHI/ShaderStage.hpp"
#include "Engine/Config/EngineConfig.hpp"

class ComputeShaderStage : public ShaderStage
{
public:
	ID3D11ComputeShader* m_dx_compute_shader;
	ID3D11InputLayout*	m_dx_input_layout;

public:
	ComputeShaderStage(RHIDevice* rhi_device, 
					  const char* shader_raw_source, 
					  const size_t shader_raw_source_size, 
					  const char* entry_point = DEFAULT_COMPUTE_SHADER_ENTRY_POINT);

	virtual ~ComputeShaderStage() override;

	bool is_valid() const { return m_dx_compute_shader != nullptr; }
};
