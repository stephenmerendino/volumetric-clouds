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
	float x = data.texCoord.x * 5.0;
	float y = data.texCoord.y * 7.0;

	float marbleness = 5;

	float noise = Compute2dPerlinNoiseZeroToOne(x, y, 1.0, 4, 0.5, 2.5, true, 0) * marbleness;

	float diffuse = abs(sin(x + y + noise));
	diffuse = sqrt(sqrt(diffuse));

	if (diffuse < 0.3){
		diffuse = 0.3;
	}

	return float4(diffuse, diffuse, diffuse, 1.0);
}