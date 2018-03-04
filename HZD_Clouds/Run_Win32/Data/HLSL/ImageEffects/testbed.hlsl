#include "noise.h"
#include "hsv.h"

Texture2D <float4> tImage : register(t0);
SamplerState sSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	float4x4 MODEL;
	float4x4 VIEW;
	float4x4 PROJECTION;
}

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

	float4 worldSpaceVertex = mul(float4(vertex.position, 1.0f), MODEL);
	float4 cameraSpaceVertex = mul(float4(worldSpaceVertex), VIEW);
	float4 clipSpaceVertex = mul(float4(cameraSpaceVertex), PROJECTION);

	out_data.position = clipSpaceVertex;
	out_data.texCoord = vertex.texCoord;

	return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float xDisp = Compute3dPerlinNoiseNegOneToOne(data.texCoord.x, data.texCoord.y, GAME_TIME, 0) * 0.2f;
	float yDisp = Compute3dPerlinNoiseNegOneToOne(GAME_TIME, data.texCoord.y, data.texCoord.x, 0) * 0.2f;
	float zoomLevel = Compute3dPerlinNoiseZeroToOne(GAME_TIME, yDisp, xDisp, 0) * 0.5f;

	float2 uvCenterAspectCorrect;
	uvCenterAspectCorrect.x = (data.texCoord.x - 0.5f + xDisp) * (16.0 / 9.0);
	uvCenterAspectCorrect.y = data.texCoord.y - 0.5f + yDisp;

	float distanceToCenter = sqrt(dot(uvCenterAspectCorrect, uvCenterAspectCorrect));

	float scopeSize = 0.1f + zoomLevel;

	if (distanceToCenter > scopeSize){
		float lightness = 1.0;
		lightness = sin(-1.0f * (data.texCoord.y * 1000.0f) + (GAME_TIME * 10.0));
		lightness *= 0.5;
		lightness += 1.0;
		lightness = min(lightness, 1.0);
		float4 diffuse =  tImage.Sample(sSampler, data.texCoord);
		return diffuse * float4(lightness, lightness, lightness, 1.0);
	}
	else if (distanceToCenter >= (scopeSize - 0.002f)){
		return float4(1, 0, 0, 1);
	}
	else{
		// scale distance from center and sample with that?
		data.texCoord -= float2(0.5f, 0.5f);
		data.texCoord += float2(xDisp, yDisp);

		data.texCoord -= (data.texCoord * zoomLevel);

		data.texCoord += float2(0.5f, 0.5f);
		data.texCoord -= float2(xDisp, yDisp);
		return tImage.Sample(sSampler, data.texCoord );
	}
}