#include "Engine/RHI/RHITexture1D.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Engine.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Noise.hpp"

RHITexture1D::RHITexture1D(RHIDevice* rhiDevice)
	:RHITextureBase(rhiDevice)
	,m_dxTexture1D(nullptr)
	,m_width(0)
{
}

RHITexture1D::RHITexture1D(RHIDevice* rhiDevice, const char* filename)
    :RHITexture1D(rhiDevice)
{
    LoadFromFilenameRGBA8(filename);
}

RHITexture1D::RHITexture1D(RHIDevice* rhiDevice, unsigned int width)
    :RHITexture1D(rhiDevice)
{
    m_width = width;

	m_dxBindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
    DXGI_FORMAT dx_format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_TEXTURE1D_DESC tex_desc;
	memset(&tex_desc, 0, sizeof(tex_desc));

	tex_desc.Width = m_width;
	tex_desc.MipLevels = 1;
	tex_desc.Format = dx_format;
	tex_desc.Usage = usage;
	tex_desc.BindFlags = m_dxBindFlags;
	tex_desc.CPUAccessFlags = 0U;
	tex_desc.MiscFlags = 0U;

	ID3D11Device *dx_device = m_device->m_dxDevice;
	HRESULT hr = dx_device->CreateTexture1D(&tex_desc, nullptr, &m_dxTexture1D);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create RHITexture3D\n");

	CreateViews();
}

RHITexture1D::RHITexture1D(ID3D11Texture1D* dxTexture, ID3D11ShaderResourceView* dxShaderResourceView)
    :RHITexture1D(nullptr)
{
    m_dxTexture1D = dxTexture;
    m_dxShaderResourceView = dxShaderResourceView;
}

RHITexture1D::~RHITexture1D()
{
	DX_SAFE_RELEASE(m_dxTexture1D);
}

bool RHITexture1D::LoadFromFilenameRGBA8(const char* filename)
{
    Image* image = new Image(filename, IMAGE_LOAD_MODE_FORCE_ALPHA);
    if(image->GetHeight() > 1){
        m_width = image->GetHeight();
    }else{
        m_width = image->GetWidth();
    }

	// Setup the d3d11 description
	D3D11_TEXTURE1D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));
	textureDesc.Width = m_width;     
    textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;   
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;           
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0U;
	textureDesc.MiscFlags = 0;

	m_dxBindFlags = D3D11_BIND_SHADER_RESOURCE;

	// Setup Initial Data
	D3D11_SUBRESOURCE_DATA data;
	memset(&data, 0, sizeof(data));
	data.pSysMem = image->GetImageTexelBytes();
	data.SysMemPitch = m_width * 4;

	HRESULT result = m_device->m_dxDevice->CreateTexture1D(&textureDesc, &data, &m_dxTexture1D);
	if (SUCCEEDED(result)){
		CreateViews();
	}

    m_device->m_immediateContext->m_dxDeviceContext->GenerateMips(m_dxShaderResourceView);

    delete image;

	// If we actually generated a shader resource view, then we succeeded
	return m_dxShaderResourceView != nullptr;
}

void RHITexture1D::CreateViews()
{
	if (m_dxBindFlags & D3D11_BIND_RENDER_TARGET){
		m_device->m_dxDevice->CreateRenderTargetView(m_dxTexture1D, nullptr, &m_dxRenderTargetView);
	}

	if (m_dxBindFlags & D3D11_BIND_SHADER_RESOURCE){
		m_device->m_dxDevice->CreateShaderResourceView(m_dxTexture1D, nullptr, &m_dxShaderResourceView);
	}
}