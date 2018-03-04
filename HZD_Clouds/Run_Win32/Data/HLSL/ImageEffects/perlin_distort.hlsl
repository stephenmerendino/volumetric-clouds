#include "noise.h"

Texture2D <float4> tImage : register(t0);
Texture2D <float4> tNoiseTexture : register(t2);
SamplerState sSampler : register(s0);

// kind of like a struct
// max size: 4096 of float 4's, 16 KB
// Alignment: has to be aligned to a float4. Has to be a multiple of sixteen bytes large
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
	float magnitude = 0.05;

	float4 displace = tNoiseTexture.Sample(sSampler, data.texCoord);
	displace = ((displace * 2.0) - 1.0) * magnitude;

	float4 imageDiffuse = tImage.Sample(sSampler, data.texCoord + displace.xy);

	return imageDiffuse;
}