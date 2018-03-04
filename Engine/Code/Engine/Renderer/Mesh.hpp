#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Renderer/Vertex3.hpp"
#include "Engine/Core/BinaryStream.hpp"
#include <vector>

struct draw_instruction_t
{
	unsigned int start_index;
	unsigned int count;
	PrimitiveType primitive_type;
	bool uses_index_buffer;
};

class VertexBuffer;
class IndexBuffer;

class Mesh
{
public:
	std::vector<Vertex3> m_vertexes;
	std::vector<unsigned int> m_indexes;
	std::vector<draw_instruction_t> m_draw_instructions;

	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;

public:
	Mesh();
	Mesh(const std::vector<Vertex3>& vertexes, 
		 const std::vector<unsigned int> indexes, 
		 const std::vector<draw_instruction_t>& draw_instructions);
	~Mesh();

	void set_vertexes(const std::vector<Vertex3>& vertexes);
	void set_indexes(const std::vector<unsigned int> indexes);
	void set_draw_instructions(const std::vector<draw_instruction_t>& draw_instructions);

	void write(BinaryStream& stream);
	void read(BinaryStream& stream);
};