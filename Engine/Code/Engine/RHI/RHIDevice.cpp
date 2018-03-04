#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/StructuredBuffer.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/job.h"
#include "Engine/Core/log.h"

struct load_texture_job_t
{
    std::string         filename;
    Image*              image;
    RHITexture2D*       texture;
    texture_load_cb     loaded_cb;    
};

static void load_image_from_file_job(load_texture_job_t* job_data)
{
    job_data->image = new Image(job_data->filename, IMAGE_LOAD_MODE_FORCE_ALPHA);
}

static void load_texture_from_image_job(load_texture_job_t* job_data)
{
    if(job_data->image->IsValid()){
        job_data->texture->LoadFromImage(*job_data->image);
    }else{
        // log an error
        log_warningf("Image[%s] load failed!", job_data->filename.c_str());
        job_data->texture->LoadFromColor(Rgba::PINK);
    }
}

static void cleanup_resources_job(load_texture_job_t* job_data)
{
    SAFE_DELETE(job_data->image);
    SAFE_DELETE(job_data);
}

static void call_texture_loaded_cb(texture_load_cb cb, load_texture_job_t* job_data)
{
    cb(job_data->texture);
}

RHIDevice::RHIDevice(ID3D11Device* dx11Device)
	:m_dxDevice(dx11Device)
	,m_immediateContext(nullptr)
{
}

RHIDevice::~RHIDevice()
{
    delete_textures_in_database();

	// Output more detailed debug information for the device when it gets destroyed
	// Only outputs in Debug/DebugInline builds
	#if defined(RENDER_DEBUG)
		ID3D11Debug* dxDebug = nullptr;
		HRESULT hr = m_dxDevice->QueryInterface(IID_ID3D11Debug, (void**)&dxDebug);
		ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create debug interface for reporting.\n");
		dxDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		DX_SAFE_RELEASE(dxDebug);
	#endif

	DX_SAFE_RELEASE(m_dxDevice);
}

ShaderProgram* RHIDevice::CreateEmptyShader()
{
	return new ShaderProgram(this);
}

ShaderProgram* RHIDevice::CreateShaderProgramFromFiles(const char* vertex_file, const char* geometry_file, const char* fragment_file)
{
	return new ShaderProgram(this, vertex_file, geometry_file, fragment_file);
}

ShaderProgram* RHIDevice::CreateShaderFromHLSLFile(const char* shaderFilename)
{
	return new ShaderProgram(this, shaderFilename);
}

ShaderProgram* RHIDevice::CreateShaderFromText(const char* rawShaderText)
{
	ShaderProgram* program = new ShaderProgram(this);
	program->load_all_stages_from_text(rawShaderText, rawShaderText);
	if (program->is_valid()){
		return program;
	} 
	else{
		SAFE_DELETE(program);
		return nullptr;
	}
}

VertexBuffer* RHIDevice::CreateVertexBuffer(const Vertex3* vertexes, const unsigned int numVertexes, const BufferUsage bufferUsage)
{
	return new VertexBuffer(this, vertexes, numVertexes, bufferUsage);
}

IndexBuffer* RHIDevice::CreateIndexBuffer(const unsigned int* indexes, const unsigned int numIndexes, const BufferUsage bufferUsage)
{
	return new IndexBuffer(this, indexes, numIndexes, bufferUsage);
}

ConstantBuffer* RHIDevice::CreateConstantBuffer(const void* buffer, const size_t bufferSize)
{
	return new ConstantBuffer(this, buffer, bufferSize);
}

StructuredBuffer* RHIDevice::CreateStructuredBuffer(const void* buffer, const size_t object_size, const size_t object_count)
{
	return new StructuredBuffer(this, buffer, object_size, object_count);
}

RHITexture2D* RHIDevice::CreateRHITexture2D(unsigned int width, unsigned int height, ImageFormat format)
{
	return new RHITexture2D(this, width, height, format);
}

RHITexture2D* RHIDevice::FindOrCreateRHITexture2DFromFile(const char* filename)
{
    return find_or_create_texture2d_from_file(filename);
}

RHITexture2D* RHIDevice::CreateRHITexture2DFromImage(const Image& image)
{
	return new RHITexture2D(this, image);
}

RHITexture2D* RHIDevice::CreateRHITexture2DFromColor(const Rgba& color)
{
	RHITexture2D* texture = new RHITexture2D(this);
	if (texture->LoadFromColor(color)){
		return texture;
	}
	else{
		SAFE_DELETE(texture);
		return nullptr;
	}
}

RHITexture2D* RHIDevice::CreateRHITexture2DFromPerlinNoise(const unsigned int width,
														   const unsigned int height, 
														   const bool greyscale,
														   const float scale, 
														   const unsigned int numOctaves, 
														   const float octavePersistence, 
														   const float octaveScale)
{
	RHITexture2D* texture = new RHITexture2D(this);
	if(texture->LoadFromPerlinNoise(width, height, greyscale, scale, numOctaves, octavePersistence, octaveScale)){
		return texture;
	}
	else{
		SAFE_DELETE(texture);
		return nullptr;
	}
}

RHITexture2D* RHIDevice::load_rhitexture2d_async(const std::string& filename, texture_load_cb cb)
{
    // If texture is already loaded, call the callback and return it
	std::map<std::string, RHITexture2D*>::iterator found = m_texture_database.find(filename);
	if(found != m_texture_database.end()){
        if(nullptr != cb){
            cb(found->second);
        }
		return found->second;
	}
    
    // Didn't find it, so we're going to load it via the job system
    load_texture_job_t* job_data = new load_texture_job_t();
    job_data->filename = filename;
    job_data->image = nullptr;
    job_data->texture = new RHITexture2D(this);

    Job* load_image_job = job_create(JOB_TYPE_GENERIC, load_image_from_file_job, job_data);
    Job* image_to_texture_job = job_create(JOB_TYPE_RENDERING, load_texture_from_image_job, job_data);
    Job* cleanup_job = job_create(JOB_TYPE_GENERIC, cleanup_resources_job, job_data);

    if(nullptr != cb){
        Job* cb_job = job_create(JOB_TYPE_MAIN, call_texture_loaded_cb, cb, job_data);
        cb_job->depends_on(image_to_texture_job);
        cleanup_job->depends_on(cb_job);
        job_dispatch_and_release(cb_job);
    }

    image_to_texture_job->depends_on(load_image_job);
    cleanup_job->depends_on(image_to_texture_job);

    job_dispatch_and_release(load_image_job, image_to_texture_job, cleanup_job);

    m_texture_database[filename] = job_data->texture;
    return job_data->texture;
}

Sampler* RHIDevice::CreateSampler(const SamplerDescription& optionalDescription)
{
	return new Sampler(this, optionalDescription);
}

Font* RHIDevice::CreateFontFromFile(const char* filename)
{
	return new Font(this, filename);
}

RHITexture2D* RHIDevice::find_or_create_texture2d_from_file(const char* filename)
{
	std::map<std::string, RHITexture2D*>::iterator found = m_texture_database.find(filename);
	if(found != m_texture_database.end()){
		return found->second;
	}

    RHITexture2D* texture = new RHITexture2D(this, filename);
    m_texture_database[filename] = texture;
    return texture;
}

void RHIDevice::delete_textures_in_database()
{
    std::map<std::string, RHITexture2D*>::iterator it; 
    for(it = m_texture_database.begin(); it != m_texture_database.end(); it++){
        SAFE_DELETE(it->second);
    }
    m_texture_database.clear();
}