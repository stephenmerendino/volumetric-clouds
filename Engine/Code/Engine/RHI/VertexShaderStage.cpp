#include "Engine/RHI/VertexShaderStage.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Renderer/Vertex3.hpp"

VertexShaderStage::VertexShaderStage(RHIDevice* rhi_device, const char* shader_raw_source, const size_t shader_raw_source_size, const char* entry_point)
	:ShaderStage(shader_raw_source, shader_raw_source_size, entry_point, DX_VERTEX_SHADER_TARGET)
	,m_dx_vertex_shader(nullptr)
	,m_dx_input_layout(nullptr)
{
	rhi_device->m_dxDevice->CreateVertexShader(m_byte_code->GetBufferPointer(), m_byte_code->GetBufferSize(), nullptr, &m_dx_vertex_shader);
	create_input_layout(rhi_device);
}

VertexShaderStage::~VertexShaderStage()
{
	DX_SAFE_RELEASE(m_dx_vertex_shader);
	DX_SAFE_RELEASE(m_dx_input_layout);
}

void VertexShaderStage::create_input_layout(RHIDevice* rhi_device)
{
	D3D11_INPUT_ELEMENT_DESC desc[8];
	memset(desc, 0, sizeof(desc));

	desc[0].SemanticName = "POSITION";
	desc[0].SemanticIndex = 0;
	desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[0].InputSlot = 0U;
	desc[0].AlignedByteOffset = offsetof(Vertex3, m_position);
	desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[0].InstanceDataStepRate = 0U;

	desc[1].SemanticName = "TINT";
	desc[1].SemanticIndex = 0;
	desc[1].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc[1].InputSlot = 0U;
	desc[1].AlignedByteOffset = offsetof(Vertex3, m_color);
	desc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[1].InstanceDataStepRate = 0U;

	desc[2].SemanticName = "TEXCOORD";
	desc[2].SemanticIndex = 0;
	desc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	desc[2].InputSlot = 0U;
	desc[2].AlignedByteOffset = offsetof(Vertex3, m_texCoords);
	desc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[2].InstanceDataStepRate = 0U;

	desc[3].SemanticName = "NORMAL";
	desc[3].SemanticIndex = 0;
	desc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[3].InputSlot = 0U;
	desc[3].AlignedByteOffset = offsetof(Vertex3, m_normal);
	desc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[3].InstanceDataStepRate = 0U;

	desc[4].SemanticName = "TANGENT";
	desc[4].SemanticIndex = 0;
	desc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[4].InputSlot = 0U;
	desc[4].AlignedByteOffset = offsetof(Vertex3, m_tangent);
	desc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[4].InstanceDataStepRate = 0U;

	desc[5].SemanticName = "BITANGENT";
	desc[5].SemanticIndex = 0;
	desc[5].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[5].InputSlot = 0U;
	desc[5].AlignedByteOffset = offsetof(Vertex3, m_bitangent);
	desc[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[5].InstanceDataStepRate = 0U;

	desc[6].SemanticName = "BONE_INDICES";
	desc[6].SemanticIndex = 0;
	desc[6].Format = DXGI_FORMAT_R32G32B32A32_UINT; 
	desc[6].InputSlot = 0U;
	desc[6].AlignedByteOffset = offsetof(Vertex3, m_bone_indices);
	desc[6].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA; 
	desc[6].InstanceDataStepRate = 0U;

	desc[7].SemanticName = "BONE_WEIGHTS";
	desc[7].SemanticIndex = 0;
	desc[7].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; 
	desc[7].InputSlot = 0U;
	desc[7].AlignedByteOffset = offsetof(Vertex3, m_bone_weights);
	desc[7].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA; 
	desc[7].InstanceDataStepRate = 0U;

	rhi_device->m_dxDevice->CreateInputLayout(desc, ARRAYSIZE(desc), m_byte_code->GetBufferPointer(), m_byte_code->GetBufferSize(), &m_dx_input_layout);
}

//#include <vector>
//
//struct ui_vertex_t
//{
//	Vector3 pos;
//	Vector2 uv;
//	VertexLayout layout;
//};
//
//class InputLayout
//{
//	ID3D11InputLayout*	m_dx_input_layout;
//};
//
//class VertexLayout
//{
//	struct element_desc_t
//	{
//		std::string name;
//		int index;
//		Format format;
//		int input_slot;
//		int offset;
//		int instance_data_step_rate;
//	};
//
//	std::vector<element_desc_t> m_descs;
//
//	InputLayout* generate_input_layout();
//};
//
//class VertexDefinition
//{
//	std::string name;
//	VertexLayout layout;
//
//	static find_or_create(std::string name, VertexLayout layout);
//};
//
// VertexDefinition* ui = VertexDefinition::find_or_create("ui", ui_layout);
// VertexDefinition* full_layout = VertexDefinition::find_or_create("full_layout", full_layout);
//
//class Mesh
//{
//	struct submesh_t
//	{
//		VertexBuffer* vbo;
//		uint32_t offset;
//		VertexDefinition* vert_defn;
//	};
//
//	std::vector<submesh_t> m_submeshes;
//};
//
//RenderMesh(Mesh* mesh, Material* mat)
//{
//	Shader* shader = mat->shader;
//	
//}