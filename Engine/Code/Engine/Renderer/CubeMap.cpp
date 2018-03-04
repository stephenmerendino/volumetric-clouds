#include "Engine/Renderer/CubeMap.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

CubeMap::CubeMap()
	:m_texture(nullptr)
{
}

CubeMap::CubeMap(RHIDevice* device, CubeMapImagePaths cubeMapImages)
{
	LoadFromImages(device, cubeMapImages);
}

CubeMap::~CubeMap()
{
	SAFE_DELETE(m_texture);
}

bool CubeMap::LoadFromImages(RHIDevice* device, CubeMapImagePaths cubeMapImages)
{
	Image* loadedImages[6] = { nullptr };

	loadedImages[POS_X] = new Image(cubeMapImages.posX, IMAGE_LOAD_MODE_FORCE_ALPHA); 
	loadedImages[NEG_X] = new Image(cubeMapImages.negX, IMAGE_LOAD_MODE_FORCE_ALPHA);

	loadedImages[POS_Y] = new Image(cubeMapImages.posY, IMAGE_LOAD_MODE_FORCE_ALPHA);
	loadedImages[NEG_Y] = new Image(cubeMapImages.negY, IMAGE_LOAD_MODE_FORCE_ALPHA);

	loadedImages[POS_Z] = new Image(cubeMapImages.posZ, IMAGE_LOAD_MODE_FORCE_ALPHA);
	loadedImages[NEG_Z] = new Image(cubeMapImages.negZ, IMAGE_LOAD_MODE_FORCE_ALPHA);

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = loadedImages[0]->GetWidth();
	texDesc.Height = loadedImages[0]->GetHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	D3D11_SUBRESOURCE_DATA pData[6];
	for(int cubeMapFaceIndex = 0; cubeMapFaceIndex < 6; cubeMapFaceIndex++)
	{
		pData[cubeMapFaceIndex].pSysMem = loadedImages[cubeMapFaceIndex]->GetImageTexelBytes();
		pData[cubeMapFaceIndex].SysMemPitch = loadedImages[cubeMapFaceIndex]->GetWidth() * 4;
		pData[cubeMapFaceIndex].SysMemSlicePitch = 0;
	}

	ID3D11Texture2D* dxCubeTexture;
	ID3D11ShaderResourceView* dxShaderResourceView;

	// Create the cube texture
	HRESULT hr = device->m_dxDevice->CreateTexture2D(&texDesc, &pData[0], &dxCubeTexture);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create DX11Texture2D for cube map");

	// Create the shader resource view
	hr = device->m_dxDevice->CreateShaderResourceView(dxCubeTexture, &srvDesc, &dxShaderResourceView);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create shader resource view for cube map");

	// Unload the images
	for(int imageIndex = 0; imageIndex < 6; ++imageIndex){
		SAFE_DELETE(loadedImages[imageIndex]);
	}

	m_texture = new RHITexture2D(dxCubeTexture, dxShaderResourceView);

	return true;
}