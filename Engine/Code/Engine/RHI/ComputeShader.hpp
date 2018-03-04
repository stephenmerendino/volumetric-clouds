#pragma once

#include "Engine/RHI/Shader.hpp"

class ComputeShaderStage;
class RHIDevice;

class ComputeShader : public Shader
{
public:
    ComputeShaderStage* m_compute_stage;

public:
    ComputeShader(RHIDevice* device, const char* compute_shader_filename);
    ~ComputeShader();

	static ComputeShader* find_or_create(RHIDevice* device, const char* compute_shader_filename);
};