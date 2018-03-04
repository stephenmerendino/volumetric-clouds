#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include <string>

class RHIDevice;
struct ID3D11RasterizerState;

class RasterState
{
public:
	RHIDevice* m_device;
	ID3D11RasterizerState* m_dxRasterState;

public:
	RasterState(RHIDevice* owner, const std::string& cull_mode_string, const std::string& fill_mode_string);
	RasterState(RHIDevice* owner, CullMode cullMode = CULL_BACK, FillMode fillMode = FILL_SOLID);
	~RasterState();

	void setup(CullMode cullMode, FillMode fillMode);

	inline bool IsValid() const { return m_dxRasterState != nullptr; };
};