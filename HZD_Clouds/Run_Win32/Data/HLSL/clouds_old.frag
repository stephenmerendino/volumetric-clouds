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

cbuffer view_camera_buffer : register(b4)
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

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    float3 sky_blue = float3(0.5294117f, 0.774436f, 0.9215686f);

    const float HIGH_DENSITY_Y = 35000.0f;
    const float LOW_DENSITY_Y = 15000.0f;

    data.texCoord.y = 1.0f - data.texCoord.y;
    float u_mapped = (data.texCoord.x * 2.0f) - 1.0f;
    float v_mapped = (data.texCoord.y * 2.0f) - 1.0f;

    float3 right = CAMERA_TRANSFORM._11_12_13;
    float3 up = CAMERA_TRANSFORM._21_22_23;
    float3 forward = CAMERA_TRANSFORM._31_32_33;
    float3 eye_pos = CAMERA_TRANSFORM._41_42_43;

    //return float4(eye_pos, 1.0);

    float3 ray_dir = (forward * FOCAL_LENGTH) + (right * u_mapped) + (up * v_mapped);
    ray_dir = normalize(ray_dir);

    if(eye_pos.y < LOW_DENSITY_Y && ray_dir.y > 0.0f){
        float dy = LOW_DENSITY_Y - eye_pos.y;
        float ray_scalar = dy / ray_dir.y;
        eye_pos = eye_pos + (ray_dir * ray_scalar);
    }

    if(eye_pos.y > HIGH_DENSITY_Y && ray_dir.y < 0.0f){
        float dy = eye_pos.y - HIGH_DENSITY_Y;
        float ray_scalar = dy / abs(ray_dir.y);
        eye_pos = eye_pos + (ray_dir * ray_scalar);
    }

    float3 final_color = float3(1.0, 1.0, 1.0);

    int steps = 64;
    float step_size = (HIGH_DENSITY_Y - LOW_DENSITY_Y) / (float)steps;

    float test = 1.0 / (HIGH_DENSITY_Y - LOW_DENSITY_Y);

    float alpha = 0.0f;
    for(int i = 0; i < steps; ++i){
        float3 sample_pos = eye_pos + (ray_dir * i * step_size);

        if(sample_pos.y < LOW_DENSITY_Y){
            continue;
        }

        if(sample_pos.y > HIGH_DENSITY_Y){
            continue;
        }

        if(sample_pos.y < LOW_DENSITY_Y && ray_dir.y <= 0.0f){
            break;
        }

        if(sample_pos.y > HIGH_DENSITY_Y && ray_dir.y >= 0.0f){
            break;
        }

        //sample perlin noise at that pos
        float scale = 1000.0f;
        float noise = Compute3dPerlinNoiseZeroToOne(sample_pos.x, sample_pos.y, sample_pos.z, scale, 4, 2.0f, 0.5f, false, 0U);

        //multiply into density factor
        float cloud_density = ((sample_pos.y - LOW_DENSITY_Y) * test);
        float factor = cloud_density * cloud_density * cloud_density;

        float3 grey = float3(0.568627f, 0.603921f, 0.639215f) * cloud_density;
        float3 white = float3(0.92f, 0.92f, 0.92f) * (1.0f - cloud_density);

        float3 sample_color = grey + white;

        final_color = (final_color * 0.5) + (sample_color * 0.5);

        //output as alpha channel with white
        alpha += (noise * factor);

        if(alpha >= 1.0f){
            break;
        }
    }

    return float4(final_color, alpha);
}
