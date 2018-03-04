#pragma once

#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/Core/Image.hpp"
#include <map>

class RHIDevice;
class RHIOutput;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

enum ImageFormat
{
	IMAGE_FORMAT_RGBA8,
	IMAGE_FORMAT_D24S8,
	IMAGE_FORMAT_R16,
	NUM_IMAGE_FORMATS
};

class RHITexture2D : public RHITextureBase
{
public:
	ID3D11Texture2D*			m_dxTexture2D;
	unsigned int				m_width;
	unsigned int				m_height;

public:
	RHITexture2D(RHIDevice* rhiDevice);
	RHITexture2D(RHIDevice* rhiDevice, RHIOutput* rhiOutput);
	RHITexture2D(RHIDevice* rhiDevice, const Image& image);
	RHITexture2D(RHIDevice* rhiDevice, const Rgba& color);
	RHITexture2D(RHIDevice* rhiDevice, const char* filename);
	RHITexture2D(RHIDevice* rhiDevice, unsigned int width, unsigned int height, ImageFormat format, bool is_uav = false);
	RHITexture2D(ID3D11Texture2D* dxTexture, ID3D11ShaderResourceView* dxShaderResourceView);
	virtual ~RHITexture2D() override;

	unsigned int GetWidth() const { return m_width; }
	unsigned int GetHeight() const { return m_height; }

	inline bool IsValid() const { return (m_dxTexture2D != nullptr); }

	bool LoadFromFilename(const char* filename);
	bool LoadFromImage(const Image& image);
	bool LoadFromColor(const Rgba& color);

	bool LoadFromPerlinNoise(const unsigned int width, 
							 const unsigned int height, 
							 const bool greyscale = false, 
							 const float scale = 1.f, 
							 const unsigned int numOctaves = 1, 
							 const float octavePersistence = 0.5f, 
							 const float octaveScale = 2.f);

	void CreateViews();
};