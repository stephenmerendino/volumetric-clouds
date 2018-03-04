#include "Engine/RHI/FragmentShaderStage.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Renderer/Vertex3.hpp"

FragmentShaderStage::FragmentShaderStage(RHIDevice* rhi_device, const char* shader_raw_source, const size_t shader_raw_source_size, const char* entry_point)
	:ShaderStage(shader_raw_source, shader_raw_source_size, entry_point, DX_FRAGMENT_SHADER_TARGET)
	,m_dx_pixel_shader(nullptr)
	,m_dx_input_layout(nullptr)
{
	rhi_device->m_dxDevice->CreatePixelShader(m_byte_code->GetBufferPointer(), m_byte_code->GetBufferSize(), nullptr, &m_dx_pixel_shader);
}

FragmentShaderStage::~FragmentShaderStage()
{
	DX_SAFE_RELEASE(m_dx_pixel_shader);
}