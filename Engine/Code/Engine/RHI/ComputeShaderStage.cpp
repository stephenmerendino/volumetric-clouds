#include "Engine/RHI/ComputeShaderStage.hpp"
#include "Engine/RHI/RHIDevice.hpp"

ComputeShaderStage::ComputeShaderStage(RHIDevice* rhi_device, 
				  const char* shader_raw_source, 
				  const size_t shader_raw_source_size, 
				  const char* entry_point)
	:ShaderStage(shader_raw_source, shader_raw_source_size, entry_point, DX_COMPUTE_SHADER_TARGET)
	,m_dx_compute_shader(nullptr)
	,m_dx_input_layout(nullptr)
{
	rhi_device->m_dxDevice->CreateComputeShader(m_byte_code->GetBufferPointer(), m_byte_code->GetBufferSize(), nullptr, &m_dx_compute_shader);
}

ComputeShaderStage::~ComputeShaderStage()
{
    DX_SAFE_RELEASE(m_dx_compute_shader);
}