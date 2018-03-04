#include "noise.h"
#include "hsv.h"

Texture2D <float4> tImage : register(t3);
SamplerState sSampler : register(s0);

cbuffer time_cb : register(b1)
{
	float TIME;
	float3 PADDING;
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
	float x = (data.texCoord.x - 0.5) * 150.0;
	float y = (data.texCoord.y - 0.5) * 30.0;

	float woodness = 5.0;
	float noise = Compute2dPerlinNoiseZeroToOne(x, y, 4.0, 5, 0.5, 4.0, true, 0) * woodness;

	float dist = sqrt(x * x + y * y);

	float diffuse = abs(sin(dist + noise));

	float3 darkWood = float3(0.1543, 0.0329, 0.0081);
	float3 lightWood = float3(0.5453, 0.3255, 0.0103);

	float3 blend = darkWood * (1 - diffuse) + lightWood * diffuse;

	return float4(blend, 1.0);
}