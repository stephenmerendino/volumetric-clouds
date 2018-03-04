#include "noise.h"
#include "hsv.h"

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
	float hue = 190;
	float saturation = 0.0;
	float value = 1.0;

	float saturationVariance = Compute3dPerlinNoiseZeroToOne((data.texCoord.x * 15.0) + TIME, (data.texCoord.y * 15.0) + TIME, TIME * 2.0, 2.0, 8, 0.3f, 1.5f, true, 0);

	saturation += saturationVariance;

	if (saturation > 0.5){
		saturation = 1.0;
	}

	float3 rgb = ConvertHSVtoRGB(float3(hue, saturation, value));

	return float4(rgb, 1);
}