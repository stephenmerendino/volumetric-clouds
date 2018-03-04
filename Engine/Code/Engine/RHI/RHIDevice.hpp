#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/Core/Image.hpp"

class RHIDeviceContext;
class RHIOutput;
class ShaderProgram;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class StructuredBuffer;
class Vertex3;
class Font;
struct ID3D11Device;

#include "Engine/RHI/RHITexture2D.hpp"

typedef void(*texture_load_cb)(RHITexture2D* loaded_tex);

class RHIDevice
{
public:
	RHIDeviceContext*	m_immediateContext;
	ID3D11Device*		m_dxDevice;

public:
	RHIDevice(ID3D11Device* dx11Device);
	~RHIDevice();

public:
	// [OPTIONAL] Ability to create secondary outputs
	//            Neat for screensavers, tools, or utlizing a second monitor
	//RHIOutput* CreateOutput(Window *window, eRHIOutputMode const mode);
	//RHIOutput* CreateOutput(uint const px_width, uint const px_height, eRHIOutputMode const mode);

	ShaderProgram*				CreateEmptyShader();
	ShaderProgram*				CreateShaderProgramFromFiles(const char* vertex_file, const char* geometry_file, const char* fragment_file);
	ShaderProgram*				CreateShaderFromHLSLFile(const char* shaderFilename);
	ShaderProgram*				CreateShaderFromText(const char* rawShaderText);

	VertexBuffer*				CreateVertexBuffer(const Vertex3* vertexes, const unsigned int numVertexes, const BufferUsage bufferUsage = BUFFERUSAGE_STATIC);
	IndexBuffer*				CreateIndexBuffer(const unsigned int* indexes, const unsigned int numIndexes, const BufferUsage bufferUsage = BUFFERUSAGE_STATIC);
	ConstantBuffer*				CreateConstantBuffer(const void* buffer, const size_t bufferSize);
	StructuredBuffer*			CreateStructuredBuffer(const void* buffer, const size_t object_size, const size_t object_count);

	RHITexture2D*				CreateRHITexture2D(unsigned int width, unsigned int height, ImageFormat format);
	RHITexture2D*				FindOrCreateRHITexture2DFromFile(const char* filename);
	RHITexture2D*				CreateRHITexture2DFromImage(const Image& image);
	RHITexture2D*				CreateRHITexture2DFromColor(const Rgba& color);
	RHITexture2D*				CreateRHITexture2DFromPerlinNoise(const unsigned int width,
																  const unsigned int height, 
															      const bool greyscale = false,
																  const float scale = 1.f, 
																  const unsigned int numOctaves = 1, 
																  const float octavePersistence = 0.5f, 
																  const float octaveScale = 2.f);

    RHITexture2D*               load_rhitexture2d_async(const std::string& filename, texture_load_cb cb = nullptr);

	Sampler*					CreateSampler(const SamplerDescription& optionalDescription);

	Font*						CreateFontFromFile(const char* filename);

private:
	std::map<std::string, RHITexture2D*> m_texture_database;
	RHITexture2D* find_or_create_texture2d_from_file(const char* filename);
    void delete_textures_in_database();
};