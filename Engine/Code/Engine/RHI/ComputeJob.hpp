#pragma once

class ComputeShader;
class RHIDevice;
class RHITextureBase;

class ComputeJob
{
public:
    ComputeShader*  m_compute_shader;
    RHITextureBase* m_out_texture;
    unsigned int    m_grid_width;
    unsigned int    m_grid_height;
    unsigned int    m_grid_depth;

public:
    ComputeJob(RHIDevice* device, const char* compute_shader_filepath);
    ~ComputeJob();

    void set_out_texture(RHITextureBase* out_texture);
    void set_grid_dimensions(unsigned int width, unsigned int height, unsigned int depth);
};