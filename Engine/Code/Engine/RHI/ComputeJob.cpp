#include "Engine/RHI/ComputeJob.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/RHI/ComputeShader.hpp"

ComputeJob::ComputeJob(RHIDevice* device, const char* compute_shader_filepath)
    :m_compute_shader(nullptr)
    ,m_grid_width(0)
    ,m_grid_height(0)
    ,m_grid_depth(0)
{
    m_compute_shader = ComputeShader::find_or_create(device, compute_shader_filepath);
}

ComputeJob::~ComputeJob()
{
    SAFE_DELETE(m_compute_shader);
}

void ComputeJob::set_out_texture(RHITextureBase* out_texture)
{
    m_out_texture = out_texture;
}

void ComputeJob::set_grid_dimensions(unsigned int width, unsigned int height, unsigned int depth)
{
    m_grid_width = width;
    m_grid_height = height;
    m_grid_depth = depth;
}