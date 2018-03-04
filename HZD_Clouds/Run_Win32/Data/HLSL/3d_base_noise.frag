#include "Data/HLSL/Util/noise.h"

cbuffer noise_gen_parameters_buffer : register(b4)
{
    unsigned int    START_SLICE;
    float           WORLEY_SCALE;
    unsigned int    WORLEY_NUM_OCTAVES;
    float           WORLEY_OCTAVE_PERSISTENCE;

    float           WORLEY_OCTAVE_SCALE;
    float           PERLIN_SCALE;
    unsigned int    PERLIN_NUM_OCTAVES;
    float           PERLIN_OCTAVE_PERSISTENCE;

    float           PERLIN_OCTAVE_SCALE;
    float           WORLEY_ONE_SCALE;
    float           WORLEY_TWO_SCALE;
    float           WORLEY_THREE_SCALE;

    unsigned int    WORLEY_ONLY_NUM_OCTAVES;
    float           WORLEY_ONLY_OCTAVE_PERSISTENCE;
    float           WORLEY_ONLY_OCTAVE_SCALE;
    float           DETAIL_WORLEY_ONE_SCALE;

    float           DETAIL_WORLEY_TWO_SCALE;
    float           DETAIL_WORLEY_THREE_SCALE;
    unsigned int    DETAIL_WORLEY_NUM_OCTAVES;
    float           DETAIL_WORLEY_OCTAVE_PERSISTENCE;

    float           DETAIL_WORLEY_OCTAVE_SCALE;
    float           GENERATION_ALL_SCALE;
    float           GENERATION_Z_SCALE;
    float           _PADDING;

    float4          VIEW_MASK;
};

struct geometry_to_fragment_t
{
	float4 position : SV_Position;
    uint layer : SV_RenderTargetArrayIndex;
};

float range_map(float value, float in_min, float in_max, float out_min, float out_max)
{
    return ((value - in_min) / (in_max - in_min)) * (out_max - out_min) + out_min;
}

float calculate_perlin_worley(float3 pos)
{
    float worley = get_worley_noise_3d(pos, WORLEY_SCALE, WORLEY_NUM_OCTAVES, WORLEY_OCTAVE_PERSISTENCE, WORLEY_OCTAVE_SCALE, true);
    float perlin = Compute3dPerlinNoiseZeroToOneWrapped(pos.x, pos.y, pos.z, PERLIN_SCALE, PERLIN_NUM_OCTAVES, PERLIN_OCTAVE_PERSISTENCE, PERLIN_OCTAVE_SCALE, 4, true, 0U);
    //float perlin = Compute3dPerlinNoiseZeroToOne(pos.x, pos.y, pos.z, PERLIN_SCALE, PERLIN_NUM_OCTAVES, PERLIN_OCTAVE_PERSISTENCE, PERLIN_OCTAVE_SCALE, true, 0U);

    perlin = range_map(perlin, 0.00f, 1.0f, 0.0, 1.0f);

    worley *= 15.0f;
    perlin *= 20.0f;

    float final = range_map(perlin, worley, 20.0f, 0.0f, 1.0f);
    return final;
}

float calculate_worley_one(float3 pos)
{
    return 1.0f - get_worley_noise_3d(pos, WORLEY_ONE_SCALE, WORLEY_ONLY_NUM_OCTAVES, WORLEY_ONLY_OCTAVE_PERSISTENCE, WORLEY_ONLY_OCTAVE_SCALE, true);
}

float calculate_worley_two(float3 pos)
{
    return 1.0f - get_worley_noise_3d(pos, WORLEY_TWO_SCALE, WORLEY_ONLY_NUM_OCTAVES, WORLEY_ONLY_OCTAVE_PERSISTENCE, WORLEY_ONLY_OCTAVE_SCALE, true);
}

float calculate_worley_three(float3 pos)
{
    return 1.0f - get_worley_noise_3d(pos, WORLEY_THREE_SCALE, WORLEY_ONLY_NUM_OCTAVES, WORLEY_ONLY_OCTAVE_PERSISTENCE, WORLEY_ONLY_OCTAVE_SCALE, true);
}

// Needs to output float4(Perlin-Worley, Worley, Worley, Worley)
float4 FragmentFunction(geometry_to_fragment_t data) : SV_Target0
{
    // map x,y,z to 0-1 range
    float mapped_x = (data.position.x + 1.0f) * 0.5f;
    float mapped_y = (data.position.y + 1.0f) * 0.5f;
    float mapped_z = (float)data.layer / 128.0f;
    //mapped_z = range_map(mapped_z, 0.0f, 1.0f, 0.0f, 20.0f);
    float3 norm_pos = float3(mapped_x, mapped_y, mapped_z);

    norm_pos *= GENERATION_ALL_SCALE;
    norm_pos.z *= GENERATION_Z_SCALE;

    float perlin_worley = calculate_perlin_worley(norm_pos);
    float worley_one    = calculate_worley_one(norm_pos);
    float worley_two    = calculate_worley_two(norm_pos);
    float worley_three  = calculate_worley_three(norm_pos);

    return float4(perlin_worley, worley_one, worley_two, worley_three);
}
