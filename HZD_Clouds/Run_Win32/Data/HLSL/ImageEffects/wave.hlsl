Texture2D <float4> tImage : register(t0);
SamplerState sSampler : register(s0);

cbuffer TimeBuffer : register(b0)
{
	float TIME;
	float DOUBLE_TIME;
	float HALF_TIME;
	float DELTA_TIME;
};

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
	float yDisp = sin(data.texCoord.x * 15.0 + TIME) * 0.02;
	float xDisp = cos(data.texCoord.y * 25.0 + TIME) * 0.02;

	float2 uvDisplaced = data.texCoord + float2(xDisp, yDisp);

	float4 diffuse = tImage.Sample(sSampler, uvDisplaced);

	return float4(diffuse.rgb, 1.0);
}