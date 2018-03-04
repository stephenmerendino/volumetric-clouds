Texture2D <float4> tImage : register(t0);
SamplerState sSampler : register(s0);

struct vertex_in_t
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	out_data.position = float4(vertex.position, 1.0f);
	out_data.texCoord = vertex.texCoord;

	return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float4 diffuse = tImage.Sample(sSampler, data.texCoord);

	float r = 1.0 - diffuse.r;
	float g = 1.0 - diffuse.g;
	float b = 1.0 - diffuse.b;

	return float4(r, g, b, 1.0);
}