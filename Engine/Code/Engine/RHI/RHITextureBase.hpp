#pragma once

#include "Engine/Core/types.h"

class RHIDevice;
class RHIOutput;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11UnorderedAccessView;
enum DXGI_FORMAT;

class RHITextureBase
{
public:
	RHIDevice*					m_device;
	ID3D11RenderTargetView*		m_dxRenderTargetView;
	ID3D11ShaderResourceView*	m_dxShaderResourceView;
	ID3D11DepthStencilView*		m_dxDepthStencilView;
    ID3D11UnorderedAccessView*  m_dxUnorderedAccessView;
	unsigned int				m_dxBindFlags;
	DXGI_FORMAT                 m_dxFormat;

	bool						m_generate_mips;
	uint						m_mip_levels;

	RHITextureBase(RHIDevice* rhiDevice);
    virtual ~RHITextureBase();

	inline bool IsRenderTarget() const { return (m_dxRenderTargetView != nullptr); }
	inline bool IsShaderSourceView() const { return (m_dxShaderResourceView != nullptr); }
	inline bool IsDepthStencilView() const { return (m_dxDepthStencilView != nullptr); }
	inline bool IsUnorderedAccessView() const { return (m_dxUnorderedAccessView != nullptr); }
};