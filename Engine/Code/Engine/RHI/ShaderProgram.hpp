#pragma once

#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/VertexShaderStage.hpp"
#include "Engine/RHI/GeometryShaderStage.hpp"
#include "Engine/RHI/FragmentShaderStage.hpp"

class RHIDevice;

class ShaderProgram
{
public:
	VertexShaderStage* m_vertex_stage;
    GeometryShaderStage* m_geometry_stage;
	FragmentShaderStage* m_fragment_stage;

	RHIDevice* m_owner;

public:
	ShaderProgram(RHIDevice* owner);
	ShaderProgram(RHIDevice* owner, const char* single_file_shader_filename);
	ShaderProgram(RHIDevice* owner, const char* vertex_filename, const char* geometry_filename, const char* fragment_filename);
	~ShaderProgram();

	/*void load_from_raw_source(RHIDevice* device, const char* raw_vertex_text, const char* raw_fragment_text);*/
	inline bool is_valid() const { return m_vertex_stage->is_valid() && m_fragment_stage->is_valid(); }

	void load_all_from_single_file(const char* single_filename);
	void load_vertex_stage_from_file(const char* vertex_filename);
	void load_geometry_stage_from_file(const char* geometry_filename);
	void load_fragment_stage_from_file(const char* fragment_filename);

	void load_all_stages_from_text(const char* vertex_text, const char* fragment_text);
	void load_vertex_stage_from_text(const char* vertex_text);
	void load_geometry_stage_from_text(const char* geometry_text);
	void load_fragment_stage_from_text(const char* fragment_text);

	int find_bind_index_for_name(const char* bind_name) const;
};