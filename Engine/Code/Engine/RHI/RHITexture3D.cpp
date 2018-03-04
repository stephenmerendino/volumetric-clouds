#include "Engine/RHI/RHITexture3D.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Engine.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Noise.hpp"

#include "Engine/Core/FileUtils.hpp"

RHITexture3D::RHITexture3D(RHIDevice* rhiDevice)
	:RHITextureBase(rhiDevice)
	,m_dxTexture3D(nullptr)
	,m_width(0)
	,m_height(0)
    ,m_depth(0)
{
}

RHITexture3D::RHITexture3D(RHIDevice* rhiDevice, const Rgba& color)
    :RHITexture3D(rhiDevice)
{
    LoadFromColor(color);
}

RHITexture3D::RHITexture3D(RHIDevice* rhiDevice, unsigned int width, unsigned int height, unsigned int depth, bool generate_mips)
    :RHITexture3D(rhiDevice)
{
    m_width = width;
    m_height = height;
    m_depth = depth;
	m_generate_mips = generate_mips;

	m_dxBindFlags = make_bind_flags();
	D3D11_TEXTURE3D_DESC tex_desc = make_desc();
	tex_desc.Usage = D3D11_USAGE_DEFAULT; //override usage to be default since this 

	ID3D11Device *dx_device = m_device->m_dxDevice;
	HRESULT hr = dx_device->CreateTexture3D(&tex_desc, nullptr, &m_dxTexture3D);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create RHITexture3D\n");

	CreateViews();
}

RHITexture3D::RHITexture3D(ID3D11Texture3D* dxTexture, ID3D11ShaderResourceView* dxShaderResourceView)
    :RHITexture3D(nullptr)
{
    m_dxTexture3D = dxTexture;
    m_dxShaderResourceView = dxShaderResourceView;
}

RHITexture3D::~RHITexture3D()
{
	destroy_current_views();
}

bool RHITexture3D::LoadFromFilenameRGBA8(const char* filename, unsigned int width, unsigned int height, unsigned int depth)
{
	destroy_current_views();

	m_width = width;
	m_height = height;
    m_depth = depth;

	m_dxBindFlags = make_bind_flags();
	D3D11_TEXTURE3D_DESC tex_desc = make_desc();

    std::vector<unsigned char> image_data;
    LoadBinaryFileToBuffer(filename, image_data);

	// Setup Initial Data
	D3D11_SUBRESOURCE_DATA data;
	D3D11_SUBRESOURCE_DATA* data_ptr = nullptr;

	unsigned int pitch = m_width * 4;
    unsigned int depth_pitch = m_height * m_width * 4;

	if(!m_generate_mips){
		memset(&data, 0, sizeof(data));

		data.pSysMem = image_data.data();
		data.SysMemPitch = pitch;
	    data.SysMemSlicePitch = depth_pitch;
		data_ptr = &data;
	}

	HRESULT result = m_device->m_dxDevice->CreateTexture3D(&tex_desc, data_ptr, &m_dxTexture3D);
	if (SUCCEEDED(result)){
		CreateViews();
	}

	if(m_generate_mips){
		m_device->m_immediateContext->m_dxDeviceContext->UpdateSubresource(m_dxTexture3D, 0, NULL, image_data.data(), pitch, depth_pitch);
	    m_device->m_immediateContext->m_dxDeviceContext->GenerateMips(m_dxShaderResourceView);
	}

	// If we actually generated a shader resource view, then we succeeded
	return m_dxShaderResourceView != nullptr;
}

bool RHITexture3D::LoadFromColor(const Rgba& color)
{
	m_width = 1;
	m_height = 1;
	m_depth = 0;

	m_dxBindFlags = make_bind_flags();
	D3D11_TEXTURE3D_DESC tex_desc = make_desc();

	// Setup Initial Data
	D3D11_SUBRESOURCE_DATA data;
	memset(&data, 0, sizeof(data));
	data.pSysMem = &color.r;
	data.SysMemPitch = sizeof(color);

	HRESULT result = m_device->m_dxDevice->CreateTexture3D(&tex_desc, &data, &m_dxTexture3D);

	if (SUCCEEDED(result)){
		CreateViews();
		return true;
	}
	else{
		return false;
	}
}

bool RHITexture3D::LoadFromNoiseSingleChannel(unsigned int width, unsigned int height, unsigned int depth, noise_func single_noise)
{
	m_width = width;
	m_height = height;
	m_depth = depth;

	unsigned int numTexels = width * height * depth;
	unsigned int colorChannelsPerTexel = 4; // RGBA
	
	// Actually generate the noise
	float* textureNoiseData = new float[numTexels * colorChannelsPerTexel];
	for (unsigned int y = 0; y < height; ++y){
		for (unsigned int x = 0; x < width; ++x){
            for(unsigned int z = 0; z < depth; ++z){

    			int index = (z * width * height) + (y * width) + x;
    			index *= colorChannelsPerTexel;

				float noise = single_noise((float)x, (float)y, (float)z);
				textureNoiseData[index] = noise;
				textureNoiseData[index + 1] = noise;
				textureNoiseData[index + 2] = noise;
    			textureNoiseData[index + 3] = 1.0;
            }
		}
	}

	m_dxBindFlags = make_bind_flags();
	D3D11_TEXTURE3D_DESC tex_desc = make_desc();

	D3D11_SUBRESOURCE_DATA data;
	D3D11_SUBRESOURCE_DATA* data_ptr = nullptr;

	uint pitch = width * colorChannelsPerTexel;
	uint depth_pitch = height * pitch;

	if(!m_generate_mips){
		memset(&data, 0, sizeof(data));
		data.pSysMem = textureNoiseData;
		data.SysMemPitch = pitch;
	    data.SysMemSlicePitch = depth_pitch;
		data_ptr = &data;
	}

	HRESULT result = m_device->m_dxDevice->CreateTexture3D(&tex_desc, data_ptr, &m_dxTexture3D);
	if (SUCCEEDED(result)){
		CreateViews();
	}

	if(m_generate_mips){
		m_device->m_immediateContext->m_dxDeviceContext->UpdateSubresource(m_dxTexture3D, 0, NULL, textureNoiseData, pitch, depth_pitch);
	    m_device->m_immediateContext->m_dxDeviceContext->GenerateMips(m_dxShaderResourceView);
	}

	// Free up the dynamic memory we allocated to store the perlin noise in
	delete[] textureNoiseData;

	// If we actually generated a shader resource view, then we succeeded
	return m_dxShaderResourceView != nullptr;
}

#include "Engine/Thread/thread.h"

struct noise_data
{
    unsigned int w;
    unsigned int h;
    unsigned int d;

    unsigned int from_slice;
    unsigned int to_slice;

    unsigned char* data;

    noise_func r_noise;
    noise_func g_noise;
    noise_func b_noise;
    noise_func a_noise;
};

void make_some_noise(void* data)
{
    noise_data* nd = (noise_data*)data;

    for(unsigned int z = nd->from_slice; z <= nd->to_slice; ++z){
    	for (unsigned int y = 0; y < nd->h; ++y){
    		for (unsigned int x = 0; x < nd->w; ++x){
    			int index = (z * nd->w * nd->h) + (y * nd->w) + x;
    			index *= 4;

                float x_f = (float)x;
                float y_f = (float)y;
                float z_f = (float)z;

				float r = Clamp(nd->r_noise(x_f, y_f, z_f), 0.0f, 1.0f);
				float g = Clamp(nd->g_noise(x_f, y_f, z_f), 0.0f, 1.0f);
				float b = Clamp(nd->b_noise(x_f, y_f, z_f), 0.0f, 1.0f);
    			float a = Clamp(nd->a_noise(x_f, y_f, z_f), 0.0f, 1.0f);

				nd->data[index]     = (unsigned char)MapFloatToRange(r, 0.0f, 1.0f, 0.0f, 255.0f);
				nd->data[index + 1] = (unsigned char)MapFloatToRange(g, 0.0f, 1.0f, 0.0f, 255.0f);
				nd->data[index + 2] = (unsigned char)MapFloatToRange(b, 0.0f, 1.0f, 0.0f, 255.0f);
    			nd->data[index + 3] = (unsigned char)MapFloatToRange(a, 0.0f, 1.0f, 0.0f, 255.0f);

            }
		}
	}

}

#include "Engine/Profile/profiler.h"

bool RHITexture3D::LoadFromNoiseMultichannel(unsigned int width, 
                                             unsigned int height, 
                                             unsigned int depth, 
                                             noise_func r_noise, 
                                             noise_func g_noise, 
                                             noise_func b_noise, 
                                             noise_func a_noise)
{
    PROFILE_LOG_SCOPE_FUNCTION();

	m_width = width;
	m_height = height;
	m_depth = depth;

	unsigned int numTexels = width * height * depth;
	unsigned int colorChannelsPerTexel = 4; // RGBA
	
	// Actually generate the noise
	unsigned char* textureNoiseData = new unsigned char[numTexels * colorChannelsPerTexel];
    const int NUM_THREADS = 8;
    thread_handle_t threads[NUM_THREADS];

    int slices_per_thread = depth / NUM_THREADS;

    for(int i = 0; i < NUM_THREADS; i++){
        noise_data* d = new noise_data();

        d->w = width;
        d->h = height;
        d->d = depth;

        d->data = textureNoiseData;

        d->from_slice = i * slices_per_thread;
        d->to_slice = d->from_slice + slices_per_thread - 1;

        d->r_noise = r_noise;
        d->g_noise = g_noise;
        d->b_noise = b_noise;
        d->a_noise = a_noise;

        threads[i] = thread_create(make_some_noise, (void*)d);
    }

    for(int i = 0; i < NUM_THREADS; i++){
        thread_join(threads[i]);
    }

	m_dxBindFlags = make_bind_flags();
	D3D11_TEXTURE3D_DESC tex_desc = make_desc();

	// Setup Initial Data
	D3D11_SUBRESOURCE_DATA data;
	D3D11_SUBRESOURCE_DATA* data_ptr = nullptr;

	uint pitch = width * colorChannelsPerTexel;
	uint depth_pitch = height * pitch;

	if(!m_generate_mips){
		memset(&data, 0, sizeof(data));
		data.pSysMem = textureNoiseData;
		data.SysMemPitch = pitch;
	    data.SysMemSlicePitch = depth_pitch;
		data_ptr = &data;
	}

	HRESULT result = m_device->m_dxDevice->CreateTexture3D(&tex_desc, data_ptr, &m_dxTexture3D);
	if (SUCCEEDED(result)){
		CreateViews();
	}

	if(m_generate_mips){
		m_device->m_immediateContext->m_dxDeviceContext->UpdateSubresource(m_dxTexture3D, 0, NULL, textureNoiseData, pitch, depth_pitch);
	    m_device->m_immediateContext->m_dxDeviceContext->GenerateMips(m_dxShaderResourceView);
	}

	// If we actually generated a shader resource view, then we succeeded
	return m_dxShaderResourceView != nullptr;
}

void RHITexture3D::CreateViews()
{
	if (m_dxBindFlags & D3D11_BIND_RENDER_TARGET){
		m_device->m_dxDevice->CreateRenderTargetView(m_dxTexture3D, nullptr, &m_dxRenderTargetView);
	}

	if (m_dxBindFlags & D3D11_BIND_SHADER_RESOURCE){
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MostDetailedMip = 0;
		desc.Texture3D.MipLevels = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		m_device->m_dxDevice->CreateShaderResourceView(m_dxTexture3D, nullptr, &m_dxShaderResourceView);
	}
}

void RHITexture3D::save_to_file(const char* filename)
{
	UNUSED(filename);
	/* old save code, need to use staging texture method to be correct though
    // SAVE TEXTURE TO FILE
    {
        size_t size = width * height * depth * 4;

        std::vector<unsigned char> test;
        test.resize(size);
        memcpy(test.data(), textureNoiseData, size);

        SaveBufferToBinaryFile("3d_noise.texture", test);
    }

	// Free up the dynamic memory we allocated to store the perlin noise in
	delete[] textureNoiseData;
	*/

}

D3D11_TEXTURE3D_DESC RHITexture3D::make_desc()
{
    DXGI_FORMAT dx_format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_TEXTURE3D_DESC tex_desc;
	memset(&tex_desc, 0, sizeof(tex_desc));

	tex_desc.Width = m_width;
	tex_desc.Height = m_height;
    tex_desc.Depth = m_depth;
	tex_desc.Format = dx_format;
	tex_desc.BindFlags = m_dxBindFlags;
	tex_desc.CPUAccessFlags = 0U;

	if(m_generate_mips){
		tex_desc.Usage = D3D11_USAGE_DEFAULT;
		tex_desc.MipLevels = 0;
		tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	} else{
		tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
		tex_desc.MipLevels = m_mip_levels;
		tex_desc.MiscFlags = 0;
	}

	return tex_desc;
}

uint RHITexture3D::make_bind_flags()
{
	uint bind_flags = D3D11_BIND_SHADER_RESOURCE;
	if(m_generate_mips){
		bind_flags |= D3D11_BIND_RENDER_TARGET;
	}
	return bind_flags;
}

void RHITexture3D::destroy_current_views()
{
	DX_SAFE_RELEASE(m_dxTexture3D);
	m_dxDepthStencilView = nullptr;
	m_dxShaderResourceView = nullptr;
	m_dxRenderTargetView = nullptr;
	m_dxUnorderedAccessView = nullptr;
}