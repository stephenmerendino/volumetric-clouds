#pragma once

#include "Engine/RHI/DX11.hpp"

#include <string>
#include <vector>

#define INVALID_BIND_INDEX -1

enum RenderDataFormat
{
	RENDER_DATA_FORMAT_UNKNOWN = -1,
	RENDER_DATA_FORMAT_CONSTANT_BUFFER,
	RENDER_DATA_FORMAT_TEXTURE_BUFFER,
	RENDER_DATA_FORMAT_TEXTURE,
	RENDER_DATA_FORMAT_SAMPLER,
	RENDER_DATA_FORMAT_RW_BUFFER,
	RENDER_DATA_FORMAT_STRUCTURED_BUFFER,
	RENDER_DATA_FORMAT_RW_STRUCTURED_BUFFER,
	RENDER_DATA_FORMAT_BYTE_ADDRESS_BUFFER,
	RENDER_DATA_FORMAT_RW_BYTE_ADDRESS_BUFFER,
	RENDER_DATA_FORMAT_APPEND_STRUCTURED_BUFFER,
	RENDER_DATA_FORMAT_CONSUME_STRUCTURED_BUFFER,
	RENDER_DATA_FORMAT_RW_STRUCTURED_BUFFER_WITH_COUNTER,
	NUM_REDER_DATA_FORMATS
};

struct shader_resource_t
{
	std::string name;
	unsigned int bind_index;
	unsigned int bind_count;
	RenderDataFormat data_format;
};

struct property_t
{
	std::string name;
	std::string type;
	unsigned int offset;
	unsigned int size;
};

struct property_block_t
{
	std::vector<property_t> m_properties;
};

class RHIDevice;

class ShaderStage
{
public:
	ID3DBlob*						m_byte_code;
	std::vector<shader_resource_t>	m_shader_resources;

public:
	ShaderStage(const char* shader_raw_source, const size_t shader_raw_source_size, const char* entry_point, const char* target);
	virtual ~ShaderStage();

	int find_bind_index_for_name(const char* bind_name);

protected:
	void compile_shader_source_to_blob(const char* opt_filename, 
									   const void* source_code, 
									   const size_t source_code_size, 
									   const char* entry_point, 
									   const char* target);

private:
	void print_constants();
	void cache_shader_resources();
};