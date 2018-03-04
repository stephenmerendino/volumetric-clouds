#include "Data/HLSL/Util/noise.h"

Texture3D <float4> tBaseImage : register(t0);
Texture3D <float4> tDetailImage : register(t1);
SamplerState sSampler : register(s0);
SamplerState sPointSampler : register(s1);
SamplerState sOtherSampler : register(s1);

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
    float SCALE;
    float COVERAGE;
    float CLOUD_LAYER_TOP_Y;

    float CLOUD_LAYER_BOTTOM_Y;
    float3 SUN_POSITION;

    float LIGHT_STEP_SIZE;
    float DETAIL_SCALE;
    float2 _PADDING;

    float4 STRATUS_GRADIENT;
    float4 CUMULUS_GRADIENT;
    float4 CUMULONUMBIS_GRADIENT;
    float4 WIND;
}

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

float range_map(float value, float in_min, float in_max, float out_min, float out_max)
{
    float in_range = in_max - in_min;
    float out_range = out_max - out_min;
    float in_norm = (value - in_min) / in_range;
    float new_val = (in_norm * out_range) + out_min;
    return new_val;
}

float apply_gradient(float t, float4 gradient)
{
    if(t < gradient.x){
        return 0;
    }

    if(t > gradient.w){
        return 0;
    }

    if(t <= gradient.y){
        return range_map(t, gradient.x, gradient.y, 0.0, 1.0);
    }

    if(t <= gradient.z){
        return 1.0;
    }

    return range_map(t, gradient.z, gradient.w, 0.0, 1.0);
}

float mix(float a, float b, float t)
{
    return (1.0 - t) * a + (b * t);
}

float get_height_fraction(float y_pos)
{
    return range_map(y_pos, CLOUD_LAYER_BOTTOM_Y, CLOUD_LAYER_TOP_Y, 0.0f, 1.0);
}

float get_height_gradient_scalar(float y_pos)
{
    float4 strat = float4(0.05f, 0.12f, 0.16f, 0.20f);
    float4 cumulus = float4(0.05f, 0.24f, 0.50f, 0.65f);
    float4 cumolo = float4(0.05f, 0.15f, 0.8f, 0.95f);

    float frac = get_height_fraction(y_pos);
    float stratus_grad = apply_gradient(frac, strat);
    float cumulus_grad = apply_gradient(frac, cumulus);
    float cumulonumbis_grad = apply_gradient(frac, cumolo);

    return cumulonumbis_grad;

    return saturate((stratus_grad * 0.333) + (cumulus_grad * 0.333) + (cumulonumbis_grad * 0.333));
}

float sample_cloud_density(float3 pos, float mip_level)
{
    if(pos.y < CLOUD_LAYER_BOTTOM_Y || pos.y > CLOUD_LAYER_TOP_Y){
        return 0.0f;
    }

    float height_frac = get_height_fraction(pos.y);
    float height_scalar = get_height_gradient_scalar(pos.y);

    float cloud_top_offset = 500.0;
    pos += height_frac * WIND.xyz * cloud_top_offset;
    pos += (WIND.xyz + float3(0.0, 0.1, 0.0)) * GAME_TIME * WIND.w;

    float4 base_sample = tBaseImage.SampleLevel(sSampler, pos * SCALE, mip_level);

    //return base_sample.r;

    float fbm = (base_sample.g * 0.625f) + (base_sample.b * 0.25f) + (base_sample.a * 0.125f);
    float base_cloud = range_map(base_sample.r, -(1.0f - fbm), 1.0f, 0.0f, 1.0f);

    //base_cloud = base_sample.r;

    //return base_cloud;

    //multiply into density factor
    base_cloud *= height_scalar;
    base_cloud = range_map(base_cloud, 1.0f - COVERAGE, 1.0, 0.0, 1.0);
    base_cloud *= COVERAGE;

    if(base_cloud > 0.0){
        float4 detail_sample = tDetailImage.SampleLevel(sSampler, pos * DETAIL_SCALE, mip_level);
        float detail_fbm = (detail_sample.r * 0.625f) + (detail_sample.g * 0.25f) + (detail_sample.b * 0.125f);

        // Transition from wispy shapes to billowy shapes over height.
        float high_freq_noise_modifier = mix(detail_fbm , 1.0 - detail_fbm , saturate(height_frac * 10.0));

        // Erode the base cloud shape with the distorted
        // high frequency Worley noises.
        base_cloud = range_map(base_cloud, detail_fbm * 0.2 ,1.0 ,0.0 ,1.0);
    }

    base_cloud = saturate(base_cloud);

    return base_cloud;
}

float3 advance_eye_pos_to_cloud_layer(float3 eye_pos, float3 ray_dir)
{
    if(eye_pos.y < CLOUD_LAYER_BOTTOM_Y && ray_dir.y > 0.0f){
        float dy = CLOUD_LAYER_BOTTOM_Y - eye_pos.y;
        float ray_scalar = dy / ray_dir.y;
        eye_pos = eye_pos + (ray_dir * ray_scalar);
        return eye_pos;
    }
    else if(eye_pos.y > CLOUD_LAYER_TOP_Y && ray_dir.y < 0.0f){
        float dy = eye_pos.y - CLOUD_LAYER_TOP_Y;
        float ray_scalar = dy / abs(ray_dir.y);
        eye_pos = eye_pos + (ray_dir * ray_scalar);
        return eye_pos;
    }

    return eye_pos;
}

bool should_loop_continue(float3 sample_pos, float3 ray_dir){
    if(sample_pos.y < CLOUD_LAYER_BOTTOM_Y){
        return true;
    }

    if(sample_pos.y > CLOUD_LAYER_TOP_Y){
        return true;
    }

    return false;
}

bool should_loop_break(float3 sample_pos, float3 ray_dir){
    if(sample_pos.y < CLOUD_LAYER_BOTTOM_Y && ray_dir.y < 0.0f){
        return true;
    }

    if(sample_pos.y > CLOUD_LAYER_TOP_Y && ray_dir.y > 0.0f){
        return true;
    }

    return false;
}

float beer(float density)
{
    return exp(-density);
}

float powder(float density)
{
    return 1.0f - exp(-density * 2.0f);
}

float henyey_greenstein(float light_dot_eye, float g)
{
    return ((1.0f - g * g) / pow(abs(1.0 + g * g - 2.0f * g * light_dot_eye), 3.0 / 2.0)) / 4.0 * 3.1415;
}

// random vectors on the unit sphere
static const float3 noise_kernel[] =
{
	float3( 0.38051305f,  0.92453449f, -0.02111345f),
	float3(-0.50625799f, -0.03590792f, -0.86163418f),
	float3(-0.32509218f, -0.94557439f,  0.01428793f),
	float3( 0.09026238f, -0.27376545f,  0.95755165f),
	float3( 0.28128598f,  0.42443639f, -0.86065785f),
	float3(-0.16852403f,  0.14748697f,  0.97460106f)
};

float calculate_light_energy(float3 pos, float cloudDensity, float3 eye)
{
    float3 to_sun = normalize(SUN_POSITION - pos);

    float light_dot_eye = dot(eye, to_sun);

    float density_along_cone = 0.0f;
    //density_along_cone += cloudDensity;
    float cone_spread_multiplier = LIGHT_STEP_SIZE;
    for(int i = 0; i < 6; i++){
        pos += (to_sun * LIGHT_STEP_SIZE * i) + (cone_spread_multiplier * noise_kernel[i] * i);
        density_along_cone += saturate(sample_cloud_density(pos, 0));
    }

    float beer_factor = beer(density_along_cone);
    float powder_factor = powder(density_along_cone);
    float henyey_factor = henyey_greenstein(light_dot_eye, 0.2);

    henyey_factor = range_map(henyey_factor, -1.0f, 1.0f, 0.0f, 1.0f);

    return (2.0 * beer_factor * powder_factor * henyey_factor);
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	// use uv to get ndc coordinate
    data.texCoord.y = 1.0f - data.texCoord.y;
    float u_mapped = (data.texCoord.x * 2.0f) - 1.0f;
    float v_mapped = (data.texCoord.y * 2.0f) - 1.0f;

    float3 right = CAMERA_TRANSFORM._11_12_13;
    float3 up = CAMERA_TRANSFORM._21_22_23;
    float3 forward = CAMERA_TRANSFORM._31_32_33;
    float3 eye_pos = CAMERA_TRANSFORM._41_42_43;

    float3 ray_dir = (forward * FOCAL_LENGTH) + (right * u_mapped) + (up * v_mapped);
    ray_dir = normalize(ray_dir);

    eye_pos = advance_eye_pos_to_cloud_layer(eye_pos, ray_dir);

    const int steps = 128;
    float step_size = (CLOUD_LAYER_TOP_Y - CLOUD_LAYER_BOTTOM_Y) / (abs(ray_dir.y) * (float)(steps));

    float distance = length(ray_dir * steps * step_size);
    float inv_distance = 1.0f / distance;

    float total_density = 0.0;
    float light_energy = 1.0;

    float3 to_sun = normalize(SUN_POSITION - eye_pos);
    float light_dot_eye = dot(to_sun, ray_dir);

    //eye_pos += ray_dir * (steps - 1) * step_size;

    float density = 0.0f;
    float light = 0.0f;

    float4 cloud_color = float4(0.0, 0.0, 0.0, 0.0);

    float color = 1.0f;

    //eye_pos += ray_dir * (steps - 1) * step_size;

    for(int i = 0; i < steps; i++){
        //float3 sample_pos = eye_pos + (-ray_dir * i * step_size);
        float3 sample_pos = eye_pos + (ray_dir * i * step_size);
        
        if(should_loop_break(sample_pos, ray_dir)){
            break;
        }

        if(should_loop_continue(sample_pos, ray_dir)){
            continue;
        }

        float cloudDensity = saturate(sample_cloud_density(sample_pos, 0));
        if(cloudDensity > 0.0f){
            density += cloudDensity;

            float transmittance = 1.0f - (density * inv_distance);
            float light_sample = calculate_light_energy(sample_pos, cloudDensity, forward);

            light_sample = exp(-light_sample) * 1.45f;

            //float4 source = float4(float3(1.0, 0.0, 0.0) * light_sample, cloudDensity * transmittance);
            float4 source = float4(float3(1.0, 1.0, 1.0) * light_sample, cloudDensity * transmittance);
            source.rgb *= source.a;
            cloud_color = (1.0f - cloud_color.a) * source + cloud_color;
        }

        if(cloud_color.a >= 1.0f){
            break;
        }
    }

    return cloud_color;
}