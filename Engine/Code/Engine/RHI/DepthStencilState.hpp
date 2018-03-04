#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include <string>

class RHIDevice;
struct ID3D11DepthStencilState;

struct DepthStencilDesc
{
	bool m_depthWritingEnabled;
	bool m_depthTestEnabled;
	DepthTest m_depthTest;

	DepthStencilDesc()
		:m_depthWritingEnabled(true)
		,m_depthTestEnabled(true)
		,m_depthTest(DEPTH_TEST_COMPARISON_LESS)
	{
	}
};

class DepthStencilState 
{
public:
	RHIDevice* m_device;
	ID3D11DepthStencilState* m_dxDepthStencilState;

public:
	DepthStencilState(RHIDevice* owner, const std::string& write_str, const std::string& test_str);
	DepthStencilState(RHIDevice* owner, DepthStencilDesc desc);
	~DepthStencilState();

	void setup(DepthStencilDesc desc);

	inline bool IsValid() const { return m_dxDepthStencilState != nullptr; };
};