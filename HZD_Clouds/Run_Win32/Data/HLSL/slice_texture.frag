#include "Data/HLSL/Util/cb_common.h"

Texture3D <float4> tTexture : register(t0);
SamplerState sSampler : register(s0);

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

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float4 world_pos : WORLD_POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 bone_weights : BONE_WEIGHTS;
    uint4 bone_indices : BONE_INDICES;
};

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    float3 sample_pos;
    sample_pos.x = data.texCoord.x;
    sample_pos.y = data.texCoord.y;
    sample_pos.z = (float)(START_SLICE) / 127.0f;

	float4 textureColor = tTexture.SampleLevel(sSampler, sample_pos, (MIDI.SLIDER_0 * 7.0f));

    if(VIEW_MASK.r > 0.0f){
        return float4(textureColor.r, 0.0f, 0.0f, 1.0f);
    }

    if(VIEW_MASK.g > 0.0f){
        return float4(0.0f, textureColor.g, 0.0f, 1.0f);
    }

    if(VIEW_MASK.b > 0.0f){
        return float4(0.0f, 0.0f, textureColor.b, 1.0f);
    }

    if(VIEW_MASK.a > 0.0f){
        return float4(textureColor.a, textureColor.a, textureColor.a, 1.0f);
    }

    return textureColor;
}
