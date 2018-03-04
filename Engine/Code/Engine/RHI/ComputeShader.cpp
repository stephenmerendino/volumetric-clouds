#include "Engine/RHI/ComputeShader.hpp"
#include "Engine/RHI/ComputeShaderStage.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"

static void load_shader_from_file(char** out_shader_text, const char* shader_filename)
{
	std::string error_message = Stringf("Failed to load shader [file: %s]\n", shader_filename);
	ASSERT_OR_DIE(LoadBinaryFileToBuffer(out_shader_text, shader_filename), error_message);
}

static ComputeShaderStage* load_compute_stage_from_file(RHIDevice* device, const char* filename)
{
    if(nullptr == filename){
        return nullptr;
    }

	char* compute_shader_source = nullptr;
	load_shader_from_file(&compute_shader_source, filename);
	size_t raw_compute_size = strlen(compute_shader_source);
	return new ComputeShaderStage(device, compute_shader_source, raw_compute_size);
}

ComputeShader::ComputeShader(RHIDevice* device, const char* compute_shader_filename)
    :Shader(nullptr, nullptr, nullptr, nullptr)
{
    m_compute_stage = load_compute_stage_from_file(device, compute_shader_filename);
}

ComputeShader::~ComputeShader()
{
    SAFE_DELETE(m_compute_stage);
}

ComputeShader* ComputeShader::find_or_create(RHIDevice* device, const char* compute_shader_filename)
{
	std::map<std::string, Shader*>::iterator found = s_database.find(compute_shader_filename);
	if(found != s_database.end()){
		return (ComputeShader*)found->second;
	}

	ComputeShader* new_shader = new ComputeShader(device, compute_shader_filename);
    // TODO, check against failure, delete it, return null
    s_database[compute_shader_filename] = new_shader;
	return new_shader;
}