#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Core/Common.hpp"
#include <float.h>

// ------------------------------
// Sampler Description
// ------------------------------

SamplerDescription::SamplerDescription()
	:m_minMipMap(0.0f)
	,m_maxMipMap(FLT_MAX)
	,m_borderColor(Rgba::BLACK)
	,m_anistropyAmount(1U)
	,m_uWrap(WRAPMODE_WRAP)
	,m_vWrap(WRAPMODE_WRAP)
	,m_wWrap(WRAPMODE_WRAP)
	,m_minFilter(FILTER_MODE_LINEAR)
	,m_magFilter(FILTER_MODE_LINEAR)
	,m_mipFilter(FILTER_MODE_POINT)
	,m_comparisonFunc(COMPARISON_NEVER)
{
}

void SamplerDescription::SetWrapMode(WrapMode uWrap, WrapMode vWrap, WrapMode wWrap)
{
	m_uWrap = uWrap;
	m_vWrap = vWrap;
	m_wWrap = wWrap;
}

void SamplerDescription::SetWrapMode(WrapMode wrap)
{
	SetWrapMode(wrap, wrap, wrap);
}

void SamplerDescription::SetBorderColor(const Rgba& color)
{
	m_borderColor = color;
}

void SamplerDescription::SetFilter(FilterMode minFilter, FilterMode maxFilter, FilterMode mipFilter)
{
	m_minFilter = minFilter;
	m_magFilter = maxFilter;
	m_mipFilter = mipFilter;
}

void SamplerDescription::SetFilterAnisotropic(unsigned int amount)
{
	m_anistropyAmount = amount;
}

void SamplerDescription::SetComparisonFunc(SamplerComparisonFunc func)
{
	m_comparisonFunc = func;
}

Sampler::Sampler(RHIDevice *rhiDevice, const SamplerDescription& optionalDescription)
	:m_device(rhiDevice)
	,m_dxSampler(nullptr)
{
	CreateSampler(optionalDescription);
}

// ------------------------------
// Sampler
// ------------------------------

Sampler::Sampler(RHIDevice* device)
	:m_device(device)
	,m_dxSampler(nullptr)
{
}

Sampler::~Sampler()
{
	DX_SAFE_RELEASE(m_dxSampler);
}

static D3D11_FILTER DXGetFilterMode(FilterMode minFilter, FilterMode magFilter)
{
	if (minFilter == FILTER_MODE_POINT){
		if (magFilter == FILTER_MODE_POINT){
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		} else{
			return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		}
	}
	else{
		if (magFilter == FILTER_MODE_POINT){
			return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		}
		else{
			return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		}
	}
}

static D3D11_TEXTURE_ADDRESS_MODE DXGetWrapMode(WrapMode wrapMode)
{
	switch (wrapMode){
		case WRAPMODE_CLAMP:	return D3D11_TEXTURE_ADDRESS_CLAMP;		break;
		case WRAPMODE_MIRROR:	return D3D11_TEXTURE_ADDRESS_MIRROR;	break;
		case WRAPMODE_BORDER:	return D3D11_TEXTURE_ADDRESS_BORDER;	break;

		default:
		case WRAPMODE_WRAP:		return D3D11_TEXTURE_ADDRESS_WRAP;		break;
	}
}

bool Sampler::CreateSampler(const SamplerDescription& samplerDescription)
{
	if (m_dxSampler){
		DX_SAFE_RELEASE(m_dxSampler);
	}

	D3D11_SAMPLER_DESC desc;
	memset(&desc, 0, sizeof(desc));

	desc.Filter = DXGetFilterMode(samplerDescription.m_minFilter, samplerDescription.m_magFilter);
	desc.AddressU = DXGetWrapMode(samplerDescription.m_uWrap);
	desc.AddressV = DXGetWrapMode(samplerDescription.m_vWrap);
	desc.AddressW = DXGetWrapMode(samplerDescription.m_wWrap);

	FLOAT borderColorf[4];
	samplerDescription.m_borderColor.GetAsFloats(borderColorf);
	memcpy(desc.BorderColor, borderColorf, sizeof(FLOAT) * 4);

	desc.MinLOD = samplerDescription.m_minMipMap;
	desc.MaxLOD = samplerDescription.m_maxMipMap;

	desc.ComparisonFunc = DXGetSamplerComparisonFunc(samplerDescription.m_comparisonFunc);

	HRESULT hr = m_device->m_dxDevice->CreateSamplerState(&desc, &m_dxSampler);
	return SUCCEEDED(hr);
}