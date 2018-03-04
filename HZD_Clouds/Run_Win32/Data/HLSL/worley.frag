#include "Data/HLSL/Util/noise.h"

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

cbuffer view_camera_buffer : register(b3)
{
	float4x4 CAMERA_TRANSFORM;
    float FOCAL_LENGTH;
    float3 _padding;
}

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 bone_weights : BONE_WEIGHTS;
    uint4 bone_indices : BONE_INDICES;
};

float range_map(float value, float in_min, float in_max, float out_min, float out_max)
{
    return ((value - in_min) / (in_max - in_min)) * (out_max - out_min) + out_min;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    data.texCoord.y = 1.0 - data.texCoord.y;
    data.texCoord.x *= (16.0 / 9.0);

    data.texCoord *= 128.0;

    float scale = 0.1;
    //float worley = get_worley_noise_3d(float3(data.texCoord, GAME_TIME * 0.05f), scale, 4, 0.5, 2.0, true);
    float worley = 1.0f - get_worley_noise_2d(data.texCoord, scale, 1, 0.5, 2.0, true);
    return float4(worley, worley, worley, 1.0);
    //worley = range_map(worley, 0.0, 1.0, 0.0, 8.0);

    float perlin_scale = 10.0;
    data.texCoord *= (perlin_scale);
    float perlin = Compute3dPerlinNoiseZeroToOne(data.texCoord.x, data.texCoord.y, SYSTEM_TIME * 5.0f, 5, 5, 2.0, 0.5, true, 0U);
    //float perlin = Compute2dPerlinNoiseZeroToOne(data.texCoord.x, data.texCoord.y, 5, 5, 2.0, 0.5, true, 0U);
    perlin = range_map(perlin, 0.15, 1.0, 0.0, 1.0);
    //return float4(perlin, perlin, perlin, 1.0);

    worley = range_map(worley, 0.0, 1.0, 0.0, 7.5);
    perlin = range_map(perlin, 0.0, 1.0, 0.0, 20.0);
    perlin = range_map(perlin, worley, 20.0, 0.0, 1.0);

    //perlin = range_map(perlin, worley, 1.0, 0.0, 1.0);

    return float4(perlin, perlin, perlin, 1.0);

    /*
    float scale = 8.0;
    float worley = 1.0f - get_worley_noise_2d(data.texCoord, scale, 6, 0.5, 2.0, true);

    float perlin_scale = 6;
    data.texCoord *= (perlin_scale);
    float perlin = Compute2dPerlinNoiseZeroToOne(data.texCoord.x, data.texCoord.y, 1, 2, 2.0, 0.5, true, 5U);

    float final_value =  0.75 * perlin + 0.25 * worley;
    return float4(final_value, final_value, final_value, 1.0);
    */
}
