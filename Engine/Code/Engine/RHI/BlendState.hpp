#pragma once

#include <string>

class RHIDevice;
struct ID3D11BlendState;

enum BlendFactor
{
	BLEND_ONE,
	BLEND_ZERO,

	BLEND_SRC_ALPHA,
	BLEND_INV_SRC_ALPHA,

	//BLEND_SRC_COLOR,
	//BLEND_DST_COLOR
};

struct BlendStateDesc
{
	bool m_enabled;
	BlendFactor m_src_factor;
	BlendFactor m_dest_factor;

	BlendStateDesc(bool enabled = true, BlendFactor src_factor = BLEND_ONE, BlendFactor dest_factor = BLEND_ZERO)
		:m_enabled(enabled)
		, m_src_factor(src_factor)
		, m_dest_factor(dest_factor)
	{
	}
};

class BlendState
{
public:
	RHIDevice* m_device;
	ID3D11BlendState* m_dxBlendState;
	BlendStateDesc m_blend_state_desc;

public:
	BlendState(RHIDevice* owner, const std::string& color_src_str, const std::string& color_dest_str, const std::string& color_op_str,
								 const std::string& alpha_src_str, const std::string& alpha_dest_str, const std::string& alpha_op_str);
	BlendState(RHIDevice* owner, BlendStateDesc blend_state_desc);
	~BlendState();

	inline bool IsValid() const { return m_dxBlendState != nullptr; };
};