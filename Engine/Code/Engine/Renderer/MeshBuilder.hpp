#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/BinaryStream.hpp"

#define COLORS_LOADED		0b00000001
#define UVS_LOADED			0b00000010
#define NORMALS_LOADED		0b00000100
#define TANGENTS_LOADED		0b00001000
#define BITANGENTS_LOADED	0b00010000

class Vertex3;
class RHIDevice;

class MeshBuilder
{
public:
	std::vector<Vertex3> m_vertexes;
	std::vector<unsigned int> m_indexes;
	std::vector<draw_instruction_t> m_draw_instructions;

	unsigned char m_load_flags;

public:
	MeshBuilder();
	~MeshBuilder();

	void set_colors_loaded(bool loaded);
	void set_uvs_loaded(bool loaded);
	void set_normals_loaded(bool loaded);
	void set_tangents_loaded(bool loaded);
	void set_bitangents_loaded(bool loaded);

	bool are_colors_loaded();
	bool are_uvs_loaded();
	bool are_normals_loaded();
	bool are_tangents_loaded();
	bool are_bitangents_loaded();

	void begin(PrimitiveType type, bool use_index_buffer = true);
	void clear();
	void push_current_draw_instruction();

	void set_color(const Rgba& color);
	void set_uv(const float u, const float v);
	void set_uv(const Vector2& uv);
	void set_normal(const Vector3& normal);
	void set_tangent(const Vector3& tangent);
	void set_bitangent(const Vector3& bitangent);
	void set_bone_weights(const Vector4& bone_weights);
	void set_bone_indices(const UIntVector4& bone_indices);

	size_t add_vertex(const Vector2& position);
	size_t add_vertex(float x, float y, float z);
	size_t add_vertex(const Vector3& position);
	size_t add_vertexes(Vector3* positions, unsigned int count);
	size_t add_vertexes(const std::vector<Vector3>& positions);

	size_t add_index(unsigned int index);
	size_t add_indexes(unsigned int* indexes, unsigned int count);
	size_t add_indexes(const std::vector<unsigned int>& indexes);

	void end();

	void generate_mikkt_tangents(bool force_generate);

	void copy_to_mesh(Mesh* mesh);
	bool write(BinaryStream& stream);
	bool read(BinaryStream& stream);

private:
	draw_instruction_t m_current_draw_instruction;
	Vertex3 m_vertex_stamp;

	unsigned int m_start_index;
	unsigned int m_current_index;

	void reset_vertex_stamp();
};

template<>
inline
bool BinaryStream::write(const draw_instruction_t& d)
{
	return write(d.start_index) && write(d.count) && write(d.primitive_type) && write(d.uses_index_buffer);
}

template<>
inline
bool BinaryStream::read(draw_instruction_t& d)
{
	return read(d.start_index) && read(d.count) && read(d.primitive_type) && read(d.uses_index_buffer);
}