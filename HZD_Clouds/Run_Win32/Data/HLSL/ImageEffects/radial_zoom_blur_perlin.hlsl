#include "noise.h"

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
	// Radial Blur (Zoom)
	float4 diffuse = tImage.Sample(sSampler, data.texCoord);

	float samples[10] = { -0.08, -0.05, -0.03, -0.02, -0.01, 0.01, 0.02, 0.03, 0.05, 0.08 };

	// Get the displacement vector from the UV to the center of the screen (NDC)
	float2 dir = 0.5 - data.texCoord;
	float dirDist = sqrt(dot(dir, dir));

	// Normalize
	dir /= dirDist;

	float blurWidth = Compute3dPerlinNoiseZeroToOne(data.texCoord.x, data.texCoord.y, TIME, 0);
	blurWidth *= 2;

	// TODO, 1d Perlin noise	
	//float blurStrength = Compute3dPerlinNoiseZeroToOne(data.texCoord.y, TIME, data.texCoord.x, 0);
	float blurStrength = Compute1dPerlinNoiseZeroToOne(TIME, 0);
	blurStrength *= 3;

	float4 blurColors = diffuse;
	for (int i = 0; i < 10; ++i){
		blurColors += tImage.Sample(sSampler, data.texCoord + (dir * samples[i] * blurWidth));
	}

	blurColors /= 11.0;

	// weighten the blur more if farther out from the center
	float t = min(dirDist * blurStrength, 1.0);

	return (diffuse * (1 - t)) + (blurColors * (t));
}