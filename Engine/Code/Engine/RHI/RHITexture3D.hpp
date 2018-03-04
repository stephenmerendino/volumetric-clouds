#pragma once

#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/Core/Image.hpp"
#include <map>

#include "Engine/RHI/DX11.hpp"

class RHIDevice;
class RHIOutput;
struct ID3D11Texture3D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

typedef float (*noise_func)(float, float, float);

class RHITexture3D : public RHITextureBase
{
	public:
		ID3D11Texture3D*			m_dxTexture3D;
		unsigned int				m_width;
		unsigned int				m_height;
	    unsigned int                m_depth;

	public:
		RHITexture3D(RHIDevice* rhiDevice);
		RHITexture3D(RHIDevice* rhiDevice, const Rgba& color);
		RHITexture3D(RHIDevice* rhiDevice, unsigned int width, unsigned int height, unsigned int depth, bool generate_mips = false);
		RHITexture3D(ID3D11Texture3D* dxTexture, ID3D11ShaderResourceView* dxShaderResourceView);
		virtual ~RHITexture3D() override;

		unsigned int GetWidth() const { return m_width; }
		unsigned int GetHeight() const { return m_height; }
		unsigned int GetDepth() const { return m_depth; }

		inline bool IsValid() const { return (m_dxTexture3D != nullptr); }

		bool LoadFromFilenameRGBA8(const char* filename, unsigned int width, unsigned int height, unsigned int depth);
		bool LoadFromColor(const Rgba& color);

		bool LoadFromNoiseSingleChannel(unsigned int width, 
	                                    unsigned int height, 
	                                    unsigned int depth, 
	                                    noise_func single_noise);

		bool LoadFromNoiseMultichannel(unsigned int width, 
	                                   unsigned int height, 
	                                   unsigned int depth, 
	                                   noise_func r_noise, 
	                                   noise_func g_noise, 
	                                   noise_func b_noise, 
	                                   noise_func a_noise);

		void CreateViews();

	    void save_to_file(const char* filename);

		D3D11_TEXTURE3D_DESC make_desc();
		uint make_bind_flags();
		void destroy_current_views();
};