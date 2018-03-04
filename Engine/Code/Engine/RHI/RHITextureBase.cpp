#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/DX11.hpp"

RHITextureBase::RHITextureBase(RHIDevice* rhiDevice)
	:m_device(rhiDevice)
	,m_dxRenderTargetView(nullptr)
	,m_dxShaderResourceView(nullptr)
	,m_dxDepthStencilView(nullptr)
	,m_dxUnorderedAccessView(nullptr)
	,m_dxBindFlags(0)
	,m_dxFormat(DXGI_FORMAT_UNKNOWN)
	,m_generate_mips(false)
	,m_mip_levels(1)
{
}

RHITextureBase::~RHITextureBase()
{
	DX_SAFE_RELEASE(m_dxRenderTargetView)
	DX_SAFE_RELEASE(m_dxShaderResourceView);
	DX_SAFE_RELEASE(m_dxDepthStencilView);
    DX_SAFE_RELEASE(m_dxUnorderedAccessView);
}
