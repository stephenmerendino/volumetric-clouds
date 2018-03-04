#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

static CullMode get_cull_mode_from_string(const std::string& cull_mode_string)
{
	if(cull_mode_string == "front") return CULL_FRONT;
	if(cull_mode_string == "back") return CULL_BACK;
	return CULL_NONE;
}

static FillMode get_fill_mode_from_string(const std::string& fill_mode_string)
{
	if(fill_mode_string == "solid") return FILL_SOLID;
	if(fill_mode_string == "WIREFRAME") return FILL_WIREFRAME;
	return FILL_SOLID;
}

RasterState::RasterState(RHIDevice* owner, const std::string& cull_mode_string, const std::string& fill_mode_string)
	:m_device(owner)
	,m_dxRasterState(nullptr)
{
	CullMode cull_mode = get_cull_mode_from_string(cull_mode_string);
	FillMode fill_mode = get_fill_mode_from_string(fill_mode_string);
	setup(cull_mode, fill_mode);
}

RasterState::RasterState(RHIDevice* owner, CullMode cullMode, FillMode fillMode)
	:m_device(owner)
	,m_dxRasterState(nullptr)
{
	setup(cullMode, fillMode);
}

void RasterState::setup(CullMode cullMode, FillMode fillMode)
{
	D3D11_RASTERIZER_DESC desc;
	memset(&desc, 0, sizeof(desc));

	desc.FillMode = DXGetFillMode(fillMode);
	desc.CullMode = DXGetCullMode(cullMode);

	// Different than the default
	desc.FrontCounterClockwise = true;
	
	desc.AntialiasedLineEnable = false;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = true;
	desc.ScissorEnable = false;
	desc.MultisampleEnable = false;

	HRESULT hr = m_device->m_dxDevice->CreateRasterizerState(&desc, &m_dxRasterState);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create raster state.\n");
}

RasterState::~RasterState()
{
	DX_SAFE_RELEASE(m_dxRasterState);
}