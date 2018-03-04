#pragma once

#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/Core/Image.hpp"
#include <map>

class RHIDevice;
class RHIOutput;
struct ID3D11Texture1D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

class RHITexture1D : public RHITextureBase
{
public:
	ID3D11Texture1D*			m_dxTexture1D;
	unsigned int				m_width;

public:
	RHITexture1D(RHIDevice* rhiDevice);
	RHITexture1D(RHIDevice* rhiDevice, const char* filename);
	RHITexture1D(RHIDevice* rhiDevice, unsigned int width);
	RHITexture1D(ID3D11Texture1D* dxTexture, ID3D11ShaderResourceView* dxShaderResourceView);
	virtual ~RHITexture1D() override;

	unsigned int GetWidth() const { return m_width; }

	inline bool IsValid() const { return (m_dxTexture1D != nullptr); }

	bool LoadFromFilenameRGBA8(const char* filename);

	void CreateViews();
};
