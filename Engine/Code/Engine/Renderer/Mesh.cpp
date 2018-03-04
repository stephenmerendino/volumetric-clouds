#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Engine.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Mesh::Mesh()
	:m_vbo(nullptr)
	,m_ibo(nullptr)
{
}

Mesh::Mesh(const std::vector<Vertex3>& vertexes,
		   const std::vector<unsigned int> indexes,
		   const std::vector<draw_instruction_t>& draw_instructions)
{
	set_vertexes(vertexes);
	set_indexes(indexes);
	set_draw_instructions(draw_instructions);
}

Mesh::~Mesh()
{
	SAFE_DELETE(m_vbo);
	SAFE_DELETE(m_ibo);
}

void Mesh::set_vertexes(const std::vector<Vertex3>& vertexes)
{
	m_vertexes = vertexes;

	if(!m_vbo){
		if(vertexes.size() == 0){
			m_vbo = g_theRenderer->m_device->CreateVertexBuffer(nullptr, 1, BUFFERUSAGE_DYNAMIC);
		}
		else{
			m_vbo = g_theRenderer->m_device->CreateVertexBuffer(m_vertexes.data(), m_vertexes.size(), BUFFERUSAGE_DYNAMIC);
		}
	}
	else{
		m_vbo->Update(g_theRenderer->m_deviceContext, m_vertexes.data(), m_vertexes.size());
	}
}

void Mesh::set_indexes(const std::vector<unsigned int> indexes)
{
	m_indexes = indexes;

	if(!m_ibo){
		if(indexes.size() == 0){
			m_ibo = g_theRenderer->m_device->CreateIndexBuffer(nullptr, 1, BUFFERUSAGE_DYNAMIC);
		}
		else{
			m_ibo = g_theRenderer->m_device->CreateIndexBuffer(m_indexes.data(), m_indexes.size(), BUFFERUSAGE_DYNAMIC);
		}
	}
	else{
		m_ibo->Update(g_theRenderer->m_deviceContext, m_indexes.data(), m_indexes.size());
	}
}

void Mesh::set_draw_instructions(const std::vector<draw_instruction_t>& draw_instructions)
{
	m_draw_instructions = draw_instructions;
}

void Mesh::write(BinaryStream& stream)
{
	stream.m_stream_order = LITTLE_ENDIAN;

	ASSERT_OR_DIE(stream.write(m_vertexes.size()), "Failed to write vertexes size");
	for(const Vertex3& vertex : m_vertexes){
		ASSERT_OR_DIE(stream.write(vertex), "Failed to write vertex");
	}

	ASSERT_OR_DIE(stream.write(m_indexes.size()), "Failed to write indexes size");
	for(const unsigned int index : m_indexes){
		ASSERT_OR_DIE(stream.write(index), "Failed to write index");
	}

	ASSERT_OR_DIE(stream.write(m_draw_instructions.size()), "Failed to write draw instructions size");
	for(const draw_instruction_t& draw_instruction : m_draw_instructions){
		ASSERT_OR_DIE(stream.write(draw_instruction), "Failed to write draw instruction");
	}
}

void Mesh::read(BinaryStream& stream)
{
	stream.m_stream_order = LITTLE_ENDIAN;

	// read in verts
	{
		unsigned int num_vertexes;
		ASSERT_OR_DIE(stream.read(&num_vertexes, sizeof(num_vertexes)), "Failed to write num vertexes");

		m_vertexes.resize(num_vertexes);

		for(unsigned int index = 0; index < num_vertexes; ++index){
			Vertex3 vert;
			ASSERT_OR_DIE(stream.read(vert), "Failed to read vertex");
			m_vertexes[index] = vert;
		}
	}

	// read in indexes
	{
		unsigned int num_indexes;
		ASSERT_OR_DIE(stream.read(&num_indexes, sizeof(num_indexes)), "Failed to read num indexes");

		m_indexes.resize(num_indexes);

		for(unsigned int index = 0; index < num_indexes; ++index){
			unsigned int vert_index;
			ASSERT_OR_DIE(stream.read(vert_index), "Failed to read to index");
			m_indexes[index] = vert_index;
		}
	}

	// read in draw instructions
	{
		unsigned int num_draw_instructions;
		ASSERT_OR_DIE(stream.read(&num_draw_instructions, sizeof(num_draw_instructions)), "Failed to read num draw instructions");

		m_draw_instructions.resize(num_draw_instructions);

		for(unsigned int draw_instr_index = 0; draw_instr_index < num_draw_instructions; ++draw_instr_index){
			draw_instruction_t draw_instruction;
			ASSERT_OR_DIE(stream.read(draw_instruction), "Failed to read draw instruction");
			m_draw_instructions[draw_instr_index] = draw_instruction;
		}
	}

	set_vertexes(m_vertexes);
	set_indexes(m_indexes);
	set_draw_instructions(m_draw_instructions);
}