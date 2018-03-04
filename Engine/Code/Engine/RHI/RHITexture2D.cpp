#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Engine.hpp"

#include "Engine/Core/job.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Noise.hpp"

RHITexture2D::RHITexture2D(RHIDevice* rhiDevice)
    :RHITextureBase(rhiDevice)
	,m_dxTexture2D(nullptr)
    ,m_width(0)
    ,m_height(0)
{
}

RHITexture2D::RHITexture2D(RHIDevice* device, RHIOutput* output)
    :RHITextureBase(device)
	,m_dxTexture2D(nullptr)
    ,m_width(0)
    ,m_height(0)
{
	// Get the back buffer
	HRESULT hr = output->m_dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_dxTexture2D);
	ASSERT_OR_DIE(SUCCEEDED(hr), "RHIOutput failed to get back buffer with the swap chain.\n");

	D3D11_TEXTURE2D_DESC textureDesc;
	m_dxTexture2D->GetDesc(&textureDesc);
	m_width = textureDesc.Width;
	m_height = textureDesc.Height;

	m_dxBindFlags = D3D11_BIND_RENDER_TARGET;
	CreateViews();
}

RHITexture2D::RHITexture2D(RHIDevice* rhiDevice, const Image& image)
    :RHITextureBase(rhiDevice)
	,m_dxTexture2D(nullptr)
    ,m_width(0)
    ,m_height(0)
{
	LoadFromImage(image);
}

RHITexture2D::RHITexture2D(RHIDevice* rhiDevice, const Rgba& color)
    :RHITextureBase(rhiDevice)
	,m_dxTexture2D(nullptr)
    ,m_width(0)
    ,m_height(0)
{
	LoadFromColor(color);
}

RHITexture2D::RHITexture2D(RHIDevice* rhiDevice, const char* filename)
    :RHITextureBase(rhiDevice)
	,m_dxTexture2D(nullptr)
    ,m_width(0)
    ,m_height(0)
{
	LoadFromFilename(filename);
}

RHITexture2D::RHITexture2D(RHIDevice* rhiDevice, unsigned int width, unsigned int height, ImageFormat format, bool is_uav)
    :RHITextureBase(rhiDevice)
	,m_dxTexture2D(nullptr)
    ,m_width(width)
    ,m_height(height)
{
	D3D11_USAGE usage = D3D11_USAGE_DEFAULT;

	m_dxBindFlags = 0U;
	switch(format) {

		case IMAGE_FORMAT_RGBA8:
		{
			m_dxFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			m_dxBindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		} break;
		
		case IMAGE_FORMAT_D24S8:
		{
			//dx_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			m_dxFormat = DXGI_FORMAT_R24G8_TYPELESS;
			m_dxBindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		} break;

		case IMAGE_FORMAT_R16:
		{
			m_dxFormat = DXGI_FORMAT_R16_UNORM;
			m_dxBindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		} break;

		default: 
		{
			ASSERT_OR_DIE(true, "Invalid image format passed to RHITexture2D constructor\n");
		}

	};

    if(is_uav){
        m_dxBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

	D3D11_TEXTURE2D_DESC tex_desc;
	memset(&tex_desc, 0, sizeof(tex_desc));

	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Usage = usage;
	tex_desc.Format = m_dxFormat;
	tex_desc.BindFlags = m_dxBindFlags;
	tex_desc.CPUAccessFlags = 0U;
	tex_desc.MiscFlags = 0U;

	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;

	ID3D11Device *dx_device = m_device->m_dxDevice;
	HRESULT hr = dx_device->CreateTexture2D(&tex_desc, nullptr, &m_dxTexture2D);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create RHITexture2D\n");

	CreateViews();
}

RHITexture2D::RHITexture2D(ID3D11Texture2D* dxTexture, ID3D11ShaderResourceView* dxShaderResourceView)
    :RHITextureBase(nullptr)
	,m_dxTexture2D(dxTexture)
    ,m_width(0)
    ,m_height(0)
{
    m_dxShaderResourceView = dxShaderResourceView;
}

RHITexture2D::~RHITexture2D()
{
	DX_SAFE_RELEASE(m_dxTexture2D);
}

bool RHITexture2D::LoadFromFilename(const char* filename)
{
	Image* image = new Image(filename, IMAGE_LOAD_MODE_FORCE_ALPHA);
	if (!image){
		return false;
	}
	LoadFromImage(*image);
	delete image;
	return true;
}

bool RHITexture2D::LoadFromImage(const Image& image)
{
	// Now, create a texture from this
	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));

	textureDesc.Width = image.GetWidth();     // width... 
	textureDesc.Height = image.GetHeight();    // ...and height of image in pixels.
	textureDesc.MipLevels = 1;    // setting to 0 means there's a full chain (or can generate a full chain) - we're immutable, so not allowed
	textureDesc.ArraySize = 1;    // only one texture (
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;            // data is set at creation time and won't change
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // R8G8B8A8 texture
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;   // we're going to be using this texture as a shader resource
	textureDesc.CPUAccessFlags = 0U;                      // Determines how I can access this resource CPU side (IMMUTABLE, So none)
	textureDesc.MiscFlags = 0;                            // Extra Flags, of note is;

	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	m_dxBindFlags = D3D11_BIND_SHADER_RESOURCE;

	// Setup Initial Data
	D3D11_SUBRESOURCE_DATA data;
	memset(&data, 0, sizeof(data));
	data.pSysMem = image.GetImageTexelBytes();
	data.SysMemPitch = image.GetWidth() * 4;

	HRESULT result = m_device->m_dxDevice->CreateTexture2D(&textureDesc, &data, &m_dxTexture2D);

	m_width = image.GetWidth();
	m_height = image.GetHeight();

	if (SUCCEEDED(result)){
		CreateViews();
		return true;
	}
	else{
		return false;
	}
}

bool RHITexture2D::LoadFromColor(const Rgba& color)
{
	// Now, create a texture from this
	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));

	textureDesc.Width = 1;
	textureDesc.Height = 1;
	textureDesc.MipLevels = 1;    // setting to 0 means there's a full chain (or can generate a full chain) - we're immutable, so not allowed
	textureDesc.ArraySize = 1;    // only one texture (
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;            // data is set at creation time and won't change
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // R8G8B8A8 texture
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;   // we're going to be using this texture as a shader resource
	textureDesc.CPUAccessFlags = 0U;                      // Determines how I can access this resource CPU side (IMMUTABLE, So none)
	textureDesc.MiscFlags = 0;                            // Extra Flags, of note is;

	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	m_dxBindFlags = D3D11_BIND_SHADER_RESOURCE;

	// Setup Initial Data
	D3D11_SUBRESOURCE_DATA data;
	memset(&data, 0, sizeof(data));
	data.pSysMem = &color.r;
	data.SysMemPitch = sizeof(color);

	HRESULT result = m_device->m_dxDevice->CreateTexture2D(&textureDesc, &data, &m_dxTexture2D);

	m_width = 1;
	m_height = 1;

	if (SUCCEEDED(result)){
		CreateViews();
		return true;
	}
	else{
		return false;
	}

}

bool RHITexture2D::LoadFromPerlinNoise(const unsigned int width, 
									   const unsigned int height, 
									   const bool greyscale,
									   const float scale, 
									   const unsigned int numOctaves, 
									   const float octavePersistence, 
									   const float octaveScale)
{
	// Setup the d3d11 description
	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));
	textureDesc.Width = width;     
	textureDesc.Height = height;  
	textureDesc.MipLevels = 1;   
	textureDesc.ArraySize = 1;  
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;           
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // We're storing rgba each as a seperate float
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0U;
	textureDesc.MiscFlags = 0; 
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	m_dxBindFlags = D3D11_BIND_SHADER_RESOURCE;

	unsigned int numTexels = width * height;
	unsigned int colorChannelsPerTexel = 4; // RGBA
	unsigned int seed = rand(); // use a randomized seed for different output each time
	
	// Actually generate the noise
	float* textureNoiseData = new float[numTexels * colorChannelsPerTexel];
	for (unsigned int y = 0; y < height; ++y){
		for (unsigned int x = 0; x < width; ++x){

			int index = (y * width) + x;
			index *= colorChannelsPerTexel;

			if (greyscale){
				float noise = Compute2dPerlinNoiseZeroToOne((float)x, (float)y, scale, numOctaves, octavePersistence, octaveScale, true, seed);
				textureNoiseData[index] = noise;
				textureNoiseData[index + 1] = noise;
				textureNoiseData[index + 2] = noise;
			}
			else{
				// Generate a different noise value for rgb channels, using which channel as a way to randomize the output of Perlin
				for (unsigned int c = 0; c < colorChannelsPerTexel - 1; ++c){
					float zChannel = (float)c * scale;
					float noise = Compute3dPerlinNoiseZeroToOne((float)x, (float)y, zChannel, scale, numOctaves, octavePersistence, octaveScale, true, seed);
					textureNoiseData[index + c] = noise;
				}
			}

			textureNoiseData[index + 3] = 1.0;
		}
	}

	// Setup Initial Data
	D3D11_SUBRESOURCE_DATA data;
	memset(&data, 0, sizeof(data));
	data.pSysMem = textureNoiseData;
	data.SysMemPitch = width * colorChannelsPerTexel * sizeof(float);

	HRESULT result = m_device->m_dxDevice->CreateTexture2D(&textureDesc, &data, &m_dxTexture2D);
	if (SUCCEEDED(result)){
		CreateViews();
	}

	// Free up the dynamic memory we allocated to store the perlin noise in
	delete[] textureNoiseData;

	m_width = width;
	m_height = height;

	// If we actually generated a shader resource view, then we succeeded
	return m_dxShaderResourceView != nullptr;
}

void RHITexture2D::CreateViews()
{
	if (m_dxBindFlags & D3D11_BIND_RENDER_TARGET){
		m_device->m_dxDevice->CreateRenderTargetView(m_dxTexture2D, nullptr, &m_dxRenderTargetView);
	}

	if (m_dxBindFlags & D3D11_BIND_SHADER_RESOURCE){
		
		if(m_dxBindFlags & D3D11_BIND_DEPTH_STENCIL){
			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
			memset(&srv_desc, 0, sizeof(srv_desc));
			srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels = 1;
			m_device->m_dxDevice->CreateShaderResourceView(m_dxTexture2D, &srv_desc, &m_dxShaderResourceView);
		}
		else{
			m_device->m_dxDevice->CreateShaderResourceView(m_dxTexture2D, nullptr, &m_dxShaderResourceView);
		}

	}

    if(m_dxBindFlags & D3D11_BIND_UNORDERED_ACCESS){
        // todo, create the unordered access view
        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
        MemZero(&uav_desc);

        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = 0;
        uav_desc.Format = m_dxFormat;

        m_device->m_dxDevice->CreateUnorderedAccessView(m_dxTexture2D, &uav_desc, &m_dxUnorderedAccessView);
    }

	if (m_dxBindFlags & D3D11_BIND_DEPTH_STENCIL){
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		memset(&dsv_desc, 0, sizeof(dsv_desc));
		dsv_desc.Flags = 0;
		dsv_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsv_desc.Texture2D.MipSlice = 0;

		m_device->m_dxDevice->CreateDepthStencilView(m_dxTexture2D, &dsv_desc, &m_dxDepthStencilView);
	}
}