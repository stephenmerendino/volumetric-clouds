#pragma once

#include "Engine/RHI/ShaderStage.hpp"
#include "Engine/Config/EngineConfig.hpp"

class FragmentShaderStage : public ShaderStage
{
public:
	ID3D11PixelShader* m_dx_pixel_shader;
	ID3D11InputLayout*	m_dx_input_layout;

public:
	FragmentShaderStage(RHIDevice* rhi_device, 
					  const char* shader_raw_source, 
					  const size_t shader_raw_source_size, 
					  const char* entry_point = DEFAULT_FRAGMENT_SHADER_ENTRY_POINT);

	virtual ~FragmentShaderStage() override;

	bool is_valid() const { return m_dx_pixel_shader != nullptr; }
};