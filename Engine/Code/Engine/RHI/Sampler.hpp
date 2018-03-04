#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/RHI/RHITypes.hpp"

class RHIDevice;
struct ID3D11SamplerState;

enum WrapMode
{
	WRAPMODE_WRAP,
	WRAPMODE_CLAMP,
	WRAPMODE_MIRROR,
	WRAPMODE_BORDER,
	NUM_WRAP_MODES
};

// Describes this sampler (translates to a D3D11_SAMPLER_DESC)
struct SamplerDescription 
{
public:
	float m_minMipMap; // 0 being most detailed.
	float m_maxMipMap; // 0 being most detailed.  (defaults to max)
	Rgba m_borderColor; // only used if wrap mode is border.

	unsigned int m_anistropyAmount; // default one - only applies if using anistropic filtering

	WrapMode m_uWrap;       // How it wraps when U leaves the [0, 1] region.
	WrapMode m_vWrap;       // ditto for V
	WrapMode m_wWrap;       // ditto for W

	FilterMode m_minFilter; // how it behaves when a pixel contains multiple texels
	FilterMode m_magFilter; // how it behaves when a texel covers multiple pixels
	FilterMode m_mipFilter; // how it behaves when crossing mip boundaries. 

	SamplerComparisonFunc m_comparisonFunc;

public:
	// default options for a smapler
	SamplerDescription();

	// Helper Methods to make setting this cleaner
	void SetWrapMode(WrapMode uWrap, WrapMode vWrap, WrapMode wWrap);
	void SetWrapMode(WrapMode wrap); // will set all to the same
	void SetBorderColor(const Rgba& color);

	void SetFilter(FilterMode minFilter, FilterMode maxFilter, FilterMode mipFilter);
	inline void SetFilter(const FilterMode filter) { SetFilter(filter, filter, filter); }
	void SetFilterAnisotropic(unsigned int amount);

	void SetComparisonFunc(SamplerComparisonFunc func);
};

class Sampler
{
public:
	RHIDevice* m_device;
	ID3D11SamplerState* m_dxSampler;

public:
	Sampler(RHIDevice* device);
	Sampler(RHIDevice* rhiDevice, const SamplerDescription& optionalDescription);
	~Sampler();

	bool CreateSampler(const SamplerDescription& samplerDescription);

	inline bool IsValid() const { return (m_dxSampler != nullptr); }
};
