#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

static DepthTest get_depth_test_from_string(const std::string& test_str)
{
	if(test_str == "never")			return DEPTH_TEST_COMPARISON_NEVER;
	if(test_str == "less")			return DEPTH_TEST_COMPARISON_LESS;
	if(test_str == "equal")			return DEPTH_TEST_COMPARISON_EQUAL;
	if(test_str == "less_equal")	return DEPTH_TEST_COMPARISON_LESS_EQUAL;
	if(test_str == "greater")		return DEPTH_TEST_COMPARISON_GREATER;
	if(test_str == "not_equal")		return DEPTH_TEST_COMPARISON_NOT_EQUAL;
	if(test_str == "greater_equal") return DEPTH_TEST_COMPARISON_GREATER_EQUAL;
	if(test_str == "always")		return DEPTH_TEST_COMPARISON_ALWAYS;

	return DEPTH_TEST_COMPARISON_ALWAYS;
}

DepthStencilState::DepthStencilState(RHIDevice* owner, const std::string& write_str, const std::string& test_str)
	:m_device(owner)
{
	// create desc using strings
	DepthStencilDesc desc;

	desc.m_depthTestEnabled = (test_str == "false" ? false : true);
	desc.m_depthWritingEnabled = (write_str == "false" ? false : true);
	desc.m_depthTest = get_depth_test_from_string(test_str);

	setup(desc);
}

DepthStencilState::DepthStencilState(RHIDevice* owner, DepthStencilDesc desc)
	:m_device(owner)
{
	setup(desc);
}

void DepthStencilState::setup(DepthStencilDesc desc)
{
	D3D11_DEPTH_STENCIL_DESC dx11Desc;
	memset(&dx11Desc, 0, sizeof(dx11Desc));

	dx11Desc.DepthEnable = desc.m_depthWritingEnabled || desc.m_depthTestEnabled;
	dx11Desc.DepthWriteMask = desc.m_depthWritingEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	dx11Desc.DepthFunc = desc.m_depthTestEnabled ? DXGetDepthComparisonFunc(desc.m_depthTest) : D3D11_COMPARISON_ALWAYS;
	dx11Desc.StencilEnable = FALSE;

	HRESULT hr = m_device->m_dxDevice->CreateDepthStencilState(&dx11Desc, &m_dxDepthStencilState);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create depth stencil state.\n");
}

DepthStencilState::~DepthStencilState()
{
	DX_SAFE_RELEASE(m_dxDepthStencilState);
}