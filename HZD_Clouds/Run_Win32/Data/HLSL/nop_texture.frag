Texture2D <float4> tDiffuse : register(t0);
Texture2D <float4> tNormal : register(t1);
Texture2D <float4> tSpec : register(t2);

SamplerState sSampler : register(s0);

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

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float4 textureColor = tDiffuse.Sample(sSampler, data.texCoord);
    return textureColor;
}
