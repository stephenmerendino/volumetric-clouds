#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Vertex3.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Profile/Profiler.h"
#include "Engine/Core/bit.h"
#include "ThirdParty/mikkt/mikktspace.h"

#include <vector>

// ------------------------------------------------------
// mikkt interface functions
// ------------------------------------------------------

// Returns the number of faces (triangles/quads) on the mesh to be processed.
int mikkt_get_num_faces(const SMikkTSpaceContext* context)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	return mb->m_vertexes.size() / 3;
}

// Returns the number of vertices on face number iFace
// iFace is a number in the range {0, 1, ..., getNumFaces()-1}
int mikkt_get_num_vertices_of_face(const SMikkTSpaceContext* context, const int face_index)
{
	UNUSED(face_index);
	UNUSED(context);
	return 3;
}

// returns the position/normal/texcoord of the referenced face of vertex number iVert.
// iVert is in the range {0,1,2} for triangles and {0,1,2,3} for quads.
void mikkt_get_position(const SMikkTSpaceContext * context, float fvPosOut[], const int face_index, const int vert_index)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	Vertex3 vert = mb->m_vertexes.at((face_index * 3) + vert_index);

	fvPosOut[0] = vert.m_position.x;
	fvPosOut[1] = vert.m_position.y;
	fvPosOut[2] = vert.m_position.z;
}

void mikkt_get_normal(const SMikkTSpaceContext* context, float fvNormOut[], const int face_index, const int vert_index)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	Vertex3 vert = mb->m_vertexes.at((face_index * 3) + vert_index);

	fvNormOut[0] = vert.m_normal.x;
	fvNormOut[1] = vert.m_normal.y;
	fvNormOut[2] = vert.m_normal.z;
}

void mikkt_get_uv(const SMikkTSpaceContext * context, float fvTexcOut[], const int face_index, const int vert_index)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	Vertex3 vert = mb->m_vertexes.at((face_index * 3) + vert_index);

	fvTexcOut[0] = vert.m_texCoords.x;
	fvTexcOut[1] = vert.m_texCoords.y;
}

// either (or both) of the two setTSpace callbacks can be set.
// The call-back m_setTSpaceBasic() is sufficient for basic normal mapping.

// This function is used to return the tangent and fSign to the application.
// fvTangent is a unit length vector.
// For normal maps it is sufficient to use the following simplified version of the bitangent which is generated at pixel/vertex level.
// bitangent = fSign * cross(vN, tangent);
// Note that the results are returned unindexed. It is possible to generate a new index list
// But averaging/overwriting tangent spaces by using an already existing index list WILL produce INCRORRECT results.
// DO NOT! use an already existing index list.
void mikkt_set_tangent_space_basic(const SMikkTSpaceContext * context, const float fvTangent[], const float fSign, const int face_index, const int vert_index)
{
	MeshBuilder* mb = (MeshBuilder*)context->m_pUserData;
	Vertex3& vert = mb->m_vertexes.at((face_index * 3) + vert_index);

	Vector3 tangent;
	tangent.x = fvTangent[0];
	tangent.y = fvTangent[1];
	tangent.z = fvTangent[2];

	vert.m_tangent = tangent;
	vert.m_bitangent = fSign * CrossProduct(vert.m_tangent, vert.m_normal);
}


MeshBuilder::MeshBuilder()
	:m_start_index(0)
	,m_current_index(0)
	,m_load_flags(0)
{
	m_current_draw_instruction.primitive_type = PRIMITIVE_NONE;
	reset_vertex_stamp();
}

MeshBuilder::~MeshBuilder()
{
}

void MeshBuilder::set_colors_loaded(bool loaded)
{
	if(IS_BIT_SET(m_load_flags, COLORS_LOADED) == loaded){
		return;
	}

	FLIP_BIT(m_load_flags, COLORS_LOADED);
}

void MeshBuilder::set_uvs_loaded(bool loaded)
{
	if(IS_BIT_SET(m_load_flags, UVS_LOADED) == loaded){
		return;
	}

	FLIP_BIT(m_load_flags, UVS_LOADED);
}

void MeshBuilder::set_normals_loaded(bool loaded)
{
	if(IS_BIT_SET(m_load_flags, NORMALS_LOADED) == loaded){
		return;
	}

	FLIP_BIT(m_load_flags, NORMALS_LOADED);
}

void MeshBuilder::set_tangents_loaded(bool loaded)
{
	if(IS_BIT_SET(m_load_flags, TANGENTS_LOADED) == loaded){
		return;
	}

	FLIP_BIT(m_load_flags, TANGENTS_LOADED);
}

void MeshBuilder::set_bitangents_loaded(bool loaded)
{
	if(IS_BIT_SET(m_load_flags, BITANGENTS_LOADED) == loaded){
		return;
	}

	FLIP_BIT(m_load_flags, BITANGENTS_LOADED);
}

bool MeshBuilder::are_colors_loaded(){ return IS_BIT_SET(m_load_flags, COLORS_LOADED); }
bool MeshBuilder::are_uvs_loaded(){ return IS_BIT_SET(m_load_flags, UVS_LOADED); }
bool MeshBuilder::are_normals_loaded(){ return IS_BIT_SET(m_load_flags, NORMALS_LOADED); }
bool MeshBuilder::are_tangents_loaded(){ return IS_BIT_SET(m_load_flags, TANGENTS_LOADED); }
bool MeshBuilder::are_bitangents_loaded(){ return IS_BIT_SET(m_load_flags, BITANGENTS_LOADED); }

void MeshBuilder::begin(PrimitiveType type, bool use_index_buffer)
{
	if(m_current_draw_instruction.primitive_type == PRIMITIVE_NONE){
		m_current_draw_instruction.primitive_type = type;
		m_current_draw_instruction.uses_index_buffer = use_index_buffer;
		m_current_draw_instruction.start_index = m_current_index;
		return;
	}

	if(m_current_draw_instruction.primitive_type != type){
		m_draw_instructions.push_back(m_current_draw_instruction);

		m_current_draw_instruction.primitive_type = type;
		m_current_draw_instruction.uses_index_buffer = use_index_buffer;
		m_current_draw_instruction.start_index = m_current_index;
	}
}

void MeshBuilder::push_current_draw_instruction()
{
	m_draw_instructions.push_back(m_current_draw_instruction);
}

void MeshBuilder::clear()
{
	m_load_flags = 0;

	m_start_index = 0;
	m_current_index = 0;
	reset_vertex_stamp();

	m_vertexes.clear();
	m_indexes.clear();
	m_draw_instructions.clear();

	m_current_draw_instruction.primitive_type = PRIMITIVE_NONE;
	m_current_draw_instruction.count = 0;
	m_current_draw_instruction.start_index = 0;
}

void MeshBuilder::set_color(const Rgba& color){ m_vertex_stamp.m_color = color; };

void MeshBuilder::set_uv(const float u, const float v){ m_vertex_stamp.m_texCoords = Vector2(u, v); }
void MeshBuilder::set_uv(const Vector2& uv){ m_vertex_stamp.m_texCoords = uv; }

void MeshBuilder::set_normal(const Vector3& normal){ m_vertex_stamp.m_normal = normal; }
void MeshBuilder::set_tangent(const Vector3& tangent){ m_vertex_stamp.m_tangent = tangent; }
void MeshBuilder::set_bitangent(const Vector3& bitangent){ m_vertex_stamp.m_bitangent = bitangent; }
void MeshBuilder::set_bone_weights(const Vector4& bone_weights){ m_vertex_stamp.m_bone_weights = bone_weights; }
void MeshBuilder::set_bone_indices(const UIntVector4& bone_indices){ m_vertex_stamp.m_bone_indices = bone_indices; }

size_t MeshBuilder::add_vertex(const Vector2& position)
{
	return add_vertex(Vector3(position.x, position.y, 0.0f));
}

size_t MeshBuilder::add_vertex(float x, float y, float z)
{
	return add_vertex(Vector3(x, y, z));
}

size_t MeshBuilder::add_vertex(const Vector3& position)
{
	m_vertex_stamp.m_position = position;
	m_vertexes.push_back(m_vertex_stamp);

	if(!m_current_draw_instruction.uses_index_buffer){
		++m_current_index;
	}

	return m_vertexes.size() - 1;
}

size_t MeshBuilder::add_vertexes(Vector3* positions, unsigned int count)
{
	for(size_t pos_count = 0; pos_count < count; ++pos_count){
		add_vertex(positions[pos_count]);
	}

	return m_vertexes.size() - 1;
}

size_t MeshBuilder::add_vertexes(const std::vector<Vector3>& positions)
{
	for(const Vector3& position : positions){
		add_vertex(position);
	}

	return m_vertexes.size() - 1;
}

size_t MeshBuilder::add_index(unsigned int index)
{
	m_indexes.push_back(index);
	++m_current_index;
	return m_current_index;
}

size_t MeshBuilder::add_indexes(unsigned int* indexes, unsigned int count)
{
	for(size_t index_count = 0; index_count < count; ++index_count){
		add_index(indexes[index_count]);
	}

	return m_current_index;
}

size_t MeshBuilder::add_indexes(const std::vector<unsigned int>& indexes)
{
	for(const unsigned int index : indexes){
		add_index(index);
	}

	return m_current_index;
}

void MeshBuilder::end()
{
	m_current_draw_instruction.count = m_current_index - m_current_draw_instruction.start_index;
	reset_vertex_stamp();
}


void MeshBuilder::generate_mikkt_tangents(bool force_generate)
{
	if(!are_normals_loaded() || !are_uvs_loaded()){
		return;
	}

	if(are_tangents_loaded() && !force_generate){
		return;
	}

	SMikkTSpaceInterface mikkt_interface;
	mikkt_interface.m_getNormal = mikkt_get_normal;
	mikkt_interface.m_getNumFaces = mikkt_get_num_faces;
	mikkt_interface.m_getNumVerticesOfFace = mikkt_get_num_vertices_of_face;
	mikkt_interface.m_getPosition = mikkt_get_position;
	mikkt_interface.m_getTexCoord = mikkt_get_uv;
	mikkt_interface.m_setTSpaceBasic = mikkt_set_tangent_space_basic;
	mikkt_interface.m_setTSpace = nullptr;

	SMikkTSpaceContext mikkt_context;
	mikkt_context.m_pInterface = &mikkt_interface;
	mikkt_context.m_pUserData = (void*)this;

	genTangSpaceDefault(&mikkt_context);

	set_tangents_loaded(true);
	set_bitangents_loaded(true);
}

void MeshBuilder::copy_to_mesh(Mesh* mesh)
{
    PROFILE_SCOPE_FUNCTION();
	if(m_current_draw_instruction.primitive_type != PRIMITIVE_NONE){
		m_draw_instructions.push_back(m_current_draw_instruction);
	}

	mesh->set_vertexes(m_vertexes);
	mesh->set_indexes(m_indexes);
	mesh->set_draw_instructions(m_draw_instructions);
}

bool MeshBuilder::write(BinaryStream& stream)
{
	stream.m_stream_order = LITTLE_ENDIAN;

	ASSERT_OR_DIE(stream.write(m_vertexes.size()), "Failed to write vertex size");
	for(const Vertex3& vertex : m_vertexes){
		ASSERT_OR_DIE(stream.write(vertex), "Failed to write vertex");
	}

	ASSERT_OR_DIE(stream.write(m_indexes.size()), "Failed to write indexes size");
	for(const unsigned int index : m_indexes){
		ASSERT_OR_DIE(stream.write(index), "Failed to write index");
	}

	ASSERT_OR_DIE(stream.write(m_draw_instructions.size()), "Failed to write draw insturctions size");
	for(const draw_instruction_t& draw_instruction : m_draw_instructions){
		ASSERT_OR_DIE(stream.write(draw_instruction), "Failed to write draw instruction");
	}

	return true;
}

bool MeshBuilder::read(BinaryStream& stream)
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

	return true;
}

void MeshBuilder::reset_vertex_stamp()
{
	m_vertex_stamp.m_position = Vector3::ZERO;
	m_vertex_stamp.m_color = Rgba::WHITE;
	m_vertex_stamp.m_texCoords = Vector2::ZERO;
	m_vertex_stamp.m_normal = -Vector3::Z_AXIS;
	m_vertex_stamp.m_tangent = Vector3::X_AXIS;
	m_vertex_stamp.m_bitangent = Vector3::Y_AXIS;
	m_vertex_stamp.m_bone_weights = Vector4::ZERO;
	m_vertex_stamp.m_bone_indices = UIntVector4::ZERO;
}