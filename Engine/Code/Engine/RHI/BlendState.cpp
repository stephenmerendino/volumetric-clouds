#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

static D3D11_BLEND DXGetBlendFactor(const BlendFactor factor)
{
	switch(factor)
	{
		case BLEND_ONE: return D3D11_BLEND_ONE;
		case BLEND_ZERO: return D3D11_BLEND_ZERO;
		case BLEND_SRC_ALPHA: return D3D11_BLEND_SRC_ALPHA;
		case BLEND_INV_SRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
		default: ASSERT_OR_DIE(false, "Undefined type");
	}
}

static D3D11_BLEND_OP DxGetOp(const std::string& op_string)
{
	if(op_string == "add") return D3D11_BLEND_OP_ADD;
	if(op_string == "subtract") return D3D11_BLEND_OP_SUBTRACT;
	if(op_string == "reverse_subtract") return D3D11_BLEND_OP_REV_SUBTRACT;
	if(op_string == "min") return D3D11_BLEND_OP_MIN;
	if(op_string == "max") return D3D11_BLEND_OP_MAX;

	return D3D11_BLEND_OP_ADD;
}

static D3D11_BLEND get_blend_from_string(const std::string blend_string)
{
	if(blend_string == "zero")		return D3D11_BLEND_ZERO;
	if(blend_string == "one")		return D3D11_BLEND_ONE;
	if(blend_string == "src")		return D3D11_BLEND_SRC_ALPHA;
	if(blend_string == "inv_src")	return D3D11_BLEND_INV_SRC_ALPHA;
	if(blend_string == "dest")		return D3D11_BLEND_DEST_ALPHA;
	if(blend_string == "inv_dest")	return D3D11_BLEND_INV_DEST_ALPHA;

	return D3D11_BLEND_ZERO;
}

BlendState::BlendState(RHIDevice* owner, const std::string& color_src_str, const std::string& color_dest_str, const std::string& color_op_str,
										 const std::string& alpha_src_str, const std::string& alpha_dest_str, const std::string& alpha_op_str)
	:m_device(owner)
	,m_dxBlendState(nullptr)
{
	D3D11_BLEND_DESC desc;
	memset(&desc, 0, sizeof(desc));

	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;

	desc.RenderTarget[0].BlendEnable = true;

	// Color blending
	desc.RenderTarget[0].BlendOp = DxGetOp(color_op_str);
	desc.RenderTarget[0].SrcBlend = get_blend_from_string(color_src_str);
	desc.RenderTarget[0].DestBlend = get_blend_from_string(color_dest_str);

	// Alpha Blending
	desc.RenderTarget[0].BlendOpAlpha = DxGetOp(alpha_op_str);
	desc.RenderTarget[0].SrcBlendAlpha = get_blend_from_string(alpha_src_str);
	desc.RenderTarget[0].DestBlendAlpha = get_blend_from_string(alpha_dest_str);

	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT hr = m_device->m_dxDevice->CreateBlendState(&desc, &m_dxBlendState);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create raster state.\n");
}

BlendState::BlendState(RHIDevice* owner, BlendStateDesc blend_state_desc)
	:m_device(owner)
	,m_dxBlendState(nullptr)
	,m_blend_state_desc(blend_state_desc)
{
	D3D11_BLEND_DESC desc;
	memset(&desc, 0, sizeof(desc));

	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;

	desc.RenderTarget[0].BlendEnable = blend_state_desc.m_enabled;

	// Color blending
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = DXGetBlendFactor(blend_state_desc.m_src_factor);
	desc.RenderTarget[0].DestBlend = DXGetBlendFactor(blend_state_desc.m_dest_factor);

	// Alpha Blending
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT hr = m_device->m_dxDevice->CreateBlendState(&desc, &m_dxBlendState);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create raster state.\n");
}

BlendState::~BlendState()
{
	DX_SAFE_RELEASE(m_dxBlendState);
}