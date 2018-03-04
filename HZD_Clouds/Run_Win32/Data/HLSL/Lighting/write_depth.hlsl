Texture2D tImage : register(t0);
SamplerState sSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    float4x4 MODEL;
    float4x4 VIEW;
    float4x4 PROJECTION;
};

cbuffer TimeBuffer : register(b1)
{
    float GAME_TIME;
    float SYSTEM_TIME;
    float GAME_FRAME_TIME;
    float SYSTEM_FRAME_TIME;
};

struct vertex_in_t
{
    float3 position : POSITION;
    float4 tint : TINT;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

struct vertex_to_fragment_t
{
    float4 position : SV_Position;
    float3 worldPosition : WORLD_POSITION;
    float4 tint : TINT;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
    vertex_to_fragment_t out_data = (vertex_to_fragment_t) 0;

    float4 worldSpaceVertex = mul(float4(vertex.position, 1.0f), MODEL);
    float4 cameraSpaceVertex = mul(float4(worldSpaceVertex), VIEW);
    float4 clipSpaceVertex = mul(float4(cameraSpaceVertex), PROJECTION);

    out_data.position = clipSpaceVertex;
    out_data.tint = vertex.tint;
    out_data.texCoord = vertex.texCoord;

    return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    return data.position.z;
}