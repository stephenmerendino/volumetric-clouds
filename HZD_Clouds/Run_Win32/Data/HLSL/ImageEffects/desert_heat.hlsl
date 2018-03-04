#include "noise.h"
#include "hsv.h"

Texture2D <float4> tImage : register(t3);
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
	float xPeriod = 50.0;
	float yPeriod = 50.0;

	float xInput = data.texCoord.x * xPeriod;
	float yInput = data.texCoord.y * yPeriod;

	float noisePower = 20;

	float heatWave = Compute2dPerlinNoiseNegOneToOne(data.texCoord.y, TIME / 10, 2.0, 6, 0.5, 2.0, false, 0) * noisePower;

	float xyValue = yInput + heatWave;

	float diffuse = abs(sin(heatWave)) * 0.003;

	float4 imageDiffuse = tImage.Sample(sSampler, data.texCoord + float2(diffuse, 0.0));

	return imageDiffuse;
}