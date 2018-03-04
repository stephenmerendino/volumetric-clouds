cbuffer MatrixBuffer : register(b0)
{
	float4x4 MODEL;
	float4x4 VIEW;
	float4x4 PROJECTION;
}

struct vertex_in_t
{
	float3 position : POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 bone_weights : BONE_WEIGHTS;
    uint4 bone_indices : BONE_INDICES;
};

struct vertex_out_t
{
	float4 position : SV_Position;
	float4 world_pos : WORLD_POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 bone_weights : BONE_WEIGHTS;
    uint4 bone_indices : BONE_INDICES;
};

vertex_out_t VertexFunction(vertex_in_t vertex)
{
	vertex_out_t out_data = (vertex_out_t)0;

	float4 world = mul(float4(vertex.position, 1.0f), MODEL);
    float4 view = mul(world, VIEW);
    float4 proj = mul(view, PROJECTION);

	out_data.position = proj;
	out_data.world_pos = world;
	out_data.texCoord = vertex.texCoord;
    out_data.tint = vertex.tint;
    out_data.normal = vertex.normal;
    out_data.tangent = vertex.tangent;
    out_data.bitangent = vertex.bitangent;
    out_data.bone_weights = vertex.bone_weights;
    out_data.bone_indices = vertex.bone_indices;

	return out_data;
}