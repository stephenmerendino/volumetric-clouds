StructuredBuffer<float4x4> tSkinMatrices : register(t5);

cbuffer MatrixBuffer : register(b0)
{
	float4x4 MODEL;
	float4x4 VIEW;
	float4x4 PROJECTION;
};

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

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 bone_weights : BONE_WEIGHTS;
    uint4 bone_indices : BONE_INDICES;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

    //skinning
    float4x4 s0 = tSkinMatrices[vertex.bone_indices.x];
    float4x4 s1 = tSkinMatrices[vertex.bone_indices.y];
    float4x4 s2 = tSkinMatrices[vertex.bone_indices.z];
    float4x4 s3 = tSkinMatrices[vertex.bone_indices.w];

    float4x4 skin = (vertex.bone_weights.x * s0) +
                    (vertex.bone_weights.y * s1) +
                    (vertex.bone_weights.z * s2) +
                    (vertex.bone_weights.w * s3);

    float4 skinned_position = mul(float4(vertex.position, 1.0), skin);

	float4 world = mul(skinned_position, MODEL);
    float4 view = mul(world, VIEW);
    float4 proj = mul(view, PROJECTION);

	out_data.position = proj;
    out_data.tint = vertex.tint;
	out_data.texCoord = vertex.texCoord;
    out_data.normal = vertex.normal;
    out_data.tangent = vertex.tangent;
    out_data.bitangent = vertex.bitangent;
    out_data.bone_weights = vertex.bone_weights;
    out_data.bone_indices = vertex.bone_indices;

	return out_data;
}
