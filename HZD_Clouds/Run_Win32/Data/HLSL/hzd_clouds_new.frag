#include "Data/HLSL/Util/noise.h"
#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/cb_common.h"

//----------------------------------------------------------
// Defines
//----------------------------------------------------------
#define M_PI 3.1415926535897932384626433832795f



//----------------------------------------------------------
// Textures
//----------------------------------------------------------

Texture3D <float4> tBaseImage : register(t0);
Texture3D <float4> tDetailImage : register(t1);



//----------------------------------------------------------
// Samplers
//----------------------------------------------------------

SamplerState sSampler : register(s0);
SamplerState sPointSampler : register(s1);



//----------------------------------------------------------
// Data Structures
//----------------------------------------------------------

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float4 world_pos : WORLD_POSITION;
	float4 tint : TINT;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 bone_weights : BONE_WEIGHTS;
    uint4 bone_indices : BONE_INDICES;
};



//----------------------------------------------------------
// Constant Buffers
//----------------------------------------------------------

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



//----------------------------------------------------------
// Helper Functions
//----------------------------------------------------------

uint ray_sphere_intersect(float3 ray_origin, float3 ray_dir, float3 sphere_origin, float sphere_radius, out float2 t)
{
    ray_dir = normalize(ray_dir);

    float3 l = ray_origin - sphere_origin;

    float a = 1.0f;
    float b = 2.0f * dot(ray_dir, l);
    float c = dot(l, l) - sphere_radius * sphere_radius;

    float discr = b * b - 4.0f * a * c;

    if(discr < 0.0f){
        t.x = t.y = 0.0f;
        return 0u;
    }
    else if (abs(discr) - 0.00005f <= 0.0f){
        t.x = t.y = -0.5f * b / a;
        return 1u;
    }
    else{
        float q = b > 0.0f ?
			-0.5f * (b + sqrt(discr)) :
			-0.5f * (b - sqrt(discr));
        float h1 = q / a;
        float h2 = c / q;
        t.x = min(h1, h2);
        t.y = max(h1, h2);
        if (t.x < 0.0f){
            t.x = t.y;
            if (t.x < 0.0f){
                return 0u;
            }
            return 1u;
        }
        return 2u;
    }
}

float2 get_ndc(float2 uv)
{
    uv.y = 1.0f - uv.y;
    float ndc_x = (uv.x * 2.0f) - 1.0f;
	ndc_x *= (16.0f / 9.0f);
    float ndc_y = (uv.y * 2.0f) - 1.0f;
	return float2(ndc_x, ndc_y);
}

float3 get_ray_dir(float2 ndc, float focal_length, float4x4 transform)
{
    float3 right = transform._11_12_13;
    float3 up = transform._21_22_23;
    float3 forward = transform._31_32_33;

	return normalize((forward * focal_length) + (right * ndc.x) + (up * ndc.y));
}

float3 get_eye_pos()
{
	return CAMERA_TRANSFORM._41_42_43;
}

float get_planet_radius()
{
    float planet_radius = 1000000.0f;
	return planet_radius;
}

float get_inner_cloud_layer_height_above_ground()
{
    float inner_cloud_layer = 1500.0f;
	return inner_cloud_layer;
}

float get_outer_cloud_layer_height_above_ground()
{
	float outer_cloud_layer = 4000.0f;
	return outer_cloud_layer;
}

float get_inner_cloud_layer_radius_from_planet_center()
{
	return get_planet_radius() + get_inner_cloud_layer_height_above_ground();
}

float get_outer_cloud_layer_radius_from_planet_center()
{
	return get_planet_radius() + get_outer_cloud_layer_height_above_ground();
}

float3 get_planet_center()
{
	float planet_radius = get_planet_radius();
    float3 planet_center = float3(0.0f, -planet_radius, 0.0f);
	return planet_center;
}

bool get_cloud_layer(in float3 eye_pos, in float3 ray_dir, out float3 cloud_layer_start, out float3 cloud_layer_end)
{
	float3 planet_center = get_planet_center();
	float inner_cloud_radius = get_inner_cloud_layer_radius_from_planet_center();
	float outer_cloud_radius = get_outer_cloud_layer_radius_from_planet_center();

    float eye_to_world_center_distance = distance(eye_pos, planet_center);

    float2 inner_t = 0.0f;
    uint inner_num_intersctions = ray_sphere_intersect(eye_pos, ray_dir, planet_center, inner_cloud_radius, inner_t);

    float2 outer_t = 0.0f;
    uint outer_num_intersctions = ray_sphere_intersect(eye_pos, ray_dir, planet_center, outer_cloud_radius, outer_t);

    float3 inner_cloud_hit = eye_pos + (ray_dir * inner_t.x);
    float3 outer_cloud_hit = eye_pos + (ray_dir * outer_t.x);

    // under the clouds
    if (eye_to_world_center_distance < inner_cloud_radius){
        // below horizon
        if (inner_cloud_hit.y < 0.0f){
            cloud_layer_start = 0.0f;
            cloud_layer_end = 0.0f;
            return false;
        }

        cloud_layer_start = inner_cloud_hit;
        cloud_layer_end = outer_cloud_hit;
        return true;
    }
    // over the clouds
    else if (eye_to_world_center_distance > outer_cloud_radius){
        // not looking down at clouds
        if (outer_num_intersctions == 0U){
            cloud_layer_start = 0.0f;
            cloud_layer_end = 0.0f;
            return false;
        }

        if (inner_num_intersctions == 0U)
        {
            cloud_layer_start = 0.0f;
            cloud_layer_end = 0.0f;
            return false;
        }

        // below horizon
        //if (outer_cloud_hit.y < 0.0f){
        //    cloud_layer_start = 0.0f;
        //    cloud_layer_end = 0.0f;
        //    return false;
        //}

        cloud_layer_start = outer_cloud_hit;
        cloud_layer_end = inner_cloud_hit;
        return true;
    }
    // in between clouds
    else{
        if (inner_num_intersctions > 0u){
            cloud_layer_end = inner_cloud_hit;
        }
        else{
            cloud_layer_end = outer_cloud_hit;
        }

        cloud_layer_start = eye_pos;
        return true;
    }
}

float get_coverage(float2 world_xz)
{
	//float scale = MIDI.SLIDER_0 * 10000.0f;
	//int num_octaves = (int)(MIDI.SLIDER_1 * 10.0f);
	//float octave_scale = MIDI.SLIDER_2 * 4.0f;
	//float octave_persistance = MIDI.SLIDER_3 * 1.0f;

	float scale = 1000.0f;
	int num_octaves = 3;
	float octave_scale = 1.5f;
	float octave_persistance = 0.5f;

	float coverage = Compute3dPerlinNoiseZeroToOne(world_xz.x, world_xz.y, GAME_TIME * 1000.0f * MIDI.KNOB_5, scale, num_octaves, octave_scale, octave_persistance, true, 0U);
	//coverage = range_map(coverage, MIDI.SLIDER_4, 1.0f, 0.0f, 1.0f);

    //if (coverage <= MIDI.SLIDER_4)
    //{
    //    coverage = range_map(coverage, MIDI.SLIDER_4, 1.0f, 0.0f, 1.0f);
    //}

	//coverage *= 1.0f - MIDI.SLIDER_4;

    coverage = saturate(coverage);

    return coverage;
}

float get_height_fraction(float3 world_pos)
{
	float3 planet_center = get_planet_center();
	float to_planet_center_dist = length(world_pos - planet_center);

	float inner_dist = get_inner_cloud_layer_radius_from_planet_center();
	float outer_dist = get_outer_cloud_layer_radius_from_planet_center();

	float percent_in_layer = range_map(to_planet_center_dist, inner_dist, outer_dist, 0.0f, 1.0f);

	return saturate(percent_in_layer);
}

float get_density_height_gradient(float height_fraction, float type)
{
	// just return cumulus for now
	float cumulus = saturate(range_map(height_fraction, 0.01f, 0.15f, 0.0f, 1.0f)) * saturate(range_map(height_fraction, 0.7f, 0.8f, 1.0f, 0.0f));
    float stratus = saturate(range_map(height_fraction, 0.05f, 0.15f, 0.0f, 1.0f)) * saturate(range_map(height_fraction, 0.4f, 0.85f, 1.0f, 0.0f));
	return stratus;
}

float sample_density(float3 world_pos, float mip_level, bool do_cheap)
{
	// get height fraction in cloud layer
	float height_fraction = get_height_fraction(world_pos);

	// wind settings
	float3 wind_direction = float3(1.0, 0.0, 0.0);
    float cloud_speed = MIDI.KNOB_6 * 1000.0;

	// push the tops of the clouds along the wind direction
	float cloud_top_offset = 500.0f;

	world_pos += height_fraction * wind_direction * cloud_top_offset;

    world_pos += (wind_direction + float3(0.0, 0.1, 0.0)  ) * GAME_TIME * cloud_speed;

	// sample base noise
	float scale = range_map(MIDI.KNOB_0, 0.0f, 1.0f, 0.0f, 0.001f);
	float4 low_freq_noises = tBaseImage.SampleLevel(sSampler, world_pos * scale, mip_level);

	// build fBm out of those
	float low_freq_fBm = (low_freq_noises.g * 0.625f) + (low_freq_noises.b * 0.25f) + (low_freq_noises.a * 0.125f);

	// define the base cloud shape by dilating it with the low frequency fBm made of Worley noise.
    float base_cloud = range_map(low_freq_noises.r, -(1.0f - low_freq_fBm), 1.0f, 0.0f, 1.0f);

	// Get the density-height gradient
    float density_height_gradient = get_density_height_gradient(height_fraction, 0.0f);

	// apply the height function to the base cloud shape
    base_cloud *= density_height_gradient;

	// get coverage
	float2 pos_xz = float2(world_pos.x, world_pos.z);
	float cloud_coverage = get_coverage(pos_xz);

	// #TODO: apply anvil deformations here to cloud coverage

	// apply coverage by dilating 
    float base_cloud_with_coverage = range_map(base_cloud, 1.0f - cloud_coverage, 1.0f, 0.0f, 1.0f); 

	// multiply result by cloud coverage so that smaller clouds are lighter and more aesthetically pleasing
    base_cloud_with_coverage *= cloud_coverage;

    //float final_cloud = saturate(base_cloud_with_coverage);
    float final_cloud = base_cloud_with_coverage;

    if (!do_cheap)
    {
        //#TODO: add some turbulence to bottoms of clouds using curl noise.  Ramp the effect down over height and scale it by some value (200 in this example)
        //float2 curl_noise = tex2Dlod(Cloud2DNoiseTexture, Cloud2DNoiseSampler, float4(float2(p.x, p.y), 0.0, 1.0).rg;
        //p.xy += curl_noise.rg * (1.0 - height_fraction) * 200.0;

        float detail_scale = range_map(MIDI.KNOB_2, 0.0f, 1.0f, 0.0f, 0.001f);
        float detail_mip = 0.0f;

        // sample high-frequency noises
        float4 high_frequency_noises = tDetailImage.SampleLevel(sSampler, world_pos * detail_scale, detail_mip).rgba;

        // build High frequency Worley noise fBm
        float high_freq_fBm = (high_frequency_noises.g * 0.625) + (high_frequency_noises.b * 0.25) + (high_frequency_noises.a * 0.125);

        // transition from wispy shapes to billowy shapes over height
        float high_freq_noise_modifier = lerp(high_freq_fBm, 1.0 - high_freq_fBm, saturate(height_fraction * 10.0));

        // erode the base cloud shape with the distorted high frequency Worley noises.
        final_cloud = range_map(final_cloud, high_freq_noise_modifier * 0.2, 1.0, 0.0, 1.0);
    }

	return final_cloud;
}

float calc_light_energy(float dl, float ds_loded, float cos_angle, float height_fraction, float phase_probability, float light_step_size, float brightness)
{
    float primary_attenuation = exp(-dl);
    float secondary_attenuation = exp(-dl * 0.25) * 0.7;
    float attenuation_probability = max(range_map(cos_angle, 0.7, 1.0, secondary_attenuation, secondary_attenuation * 0.25) , primary_attenuation);
     
    // in-scattering – one difference from presentation slides – we also reduce this effect once light has attenuated to make it directional.
    float depth_probability = lerp(0.05 + pow(saturate(ds_loded), range_map(height_fraction, 0.3, 0.85, 0.5, 2.0)), 1.0, saturate(dl / light_step_size));
    float vertical_probability = pow(saturate(range_map(height_fraction, 0.07, 0.14, 0.1, 1.0)), 0.8);
    float in_scatter_probability = depth_probability * vertical_probability;

    float light_energy = attenuation_probability * in_scatter_probability * phase_probability * brightness;

    return light_energy;
}

// lifted this out of the nubis slides from GG
float henyey_greenstein(float cos_angle, float eccentricity)
{
    return ((1.0 - eccentricity * eccentricity) / pow(abs(1.0 + eccentricity * eccentricity - 2.0 * eccentricity * cos_angle), 3.0 / 2.0)) / 4.0 * M_PI;
}

// random vectors on the unit sphere
static const float3 RANDOM_VECTORS[] =
{
    float3(0.38051305f, 0.92453449f, -0.02111345f),
	float3(-0.50625799f, -0.03590792f, -0.86163418f),
	float3(-0.32509218f, -0.94557439f, 0.01428793f),
	float3(0.09026238f, -0.27376545f, 0.95755165f),
	float3(0.28128598f, 0.42443639f, -0.86065785f),
	float3(-0.16852403f, 0.14748697f, 0.97460106f)
};

// a function to gather density in a cone for use with lighting clouds.
float sample_light_density(float3 start_pos, float3 light_pos)
{
    float step_size = MIDI.KNOB_8 * 500.0f;
    float mip_level = MIDI.KNOB_7 * 10.0f;

    float density_along_cone = 0.0;

    float3 pos = start_pos;
    float cone_radius = 100.0f;
    float3 to_light = normalize(light_pos - start_pos);

    // lighting ray march loop
    for (int i = 1; i <= 6; i++)
    {
        // add the current step offset to the sample position
        float3 cone_pos = pos + (cone_radius * RANDOM_VECTORS[i - 1] * float(i));

        // sample cloud density the expensive way
        int mip_offset = int(i * 0.5);
        density_along_cone += sample_density(cone_pos, mip_level + mip_offset, false);

        pos += (to_light * step_size);
    }

    return density_along_cone;
}

float3 calc_incident_light(float3 start_pos, float3 light_pos, float cos_angle, float absorption_coef, float3 scattering_coef)
{
    float step_size = MIDI.KNOB_8 * 500.0f;
    float mip_level = MIDI.KNOB_7 * 10.0f;

    float cone_radius = 5.0f;
    float3 to_light = normalize(light_pos - start_pos);

    float transmittance = 1.0f;
    float3 pos = start_pos;

    float total_density = 0.0f;
    
    // lighting ray march loop
    for (int i = 1; i <= 6; i++)
    {
        // add the current step offset to the sample position
        float3 cone_pos = pos + (cone_radius * RANDOM_VECTORS[i - 1] * float(i));

        // sample cloud density the expensive way
        int mip_offset = int(i * 0.5);
        float density = sample_density(cone_pos, mip_level + mip_offset, true);
        total_density += density;

        //float dt = exp(-absorption_coef * density);
        //transmittance *= dt;



        pos += (to_light * step_size);
    }

    float primary_attenuation = exp( -total_density );
    //float secondary_attenuation = exp(-total_density * 0.25) * 0.7;
    transmittance = primary_attenuation; //max( range_map( cos_angle, 0.7, 1.0, secondary_attenuation, secondary_attenuation * 0.25) , primary_attenuation);

    float3 light_color = float3(1.0, 1.0, 1.0f);

    //float eccentricity = range_map(MIDI.SLIDER_8, 0.0f, 1.0f, -1.0f, 1.0f);
    //float phase = henyey_greenstein(cos_angle, eccentricity);

    float eccentricity = MIDI.SLIDER_6;
    float silver_intensity = MIDI.SLIDER_7 * 2.0f;
    float silver_spread = MIDI.SLIDER_8;

    float phase = max(henyey_greenstein(cos_angle, eccentricity) , silver_intensity * henyey_greenstein(cos_angle, 0.99 - silver_spread));

    float3 ambient_light = float3(0.5, 0.7, 1.0f);
    float3 height_fraction = get_height_fraction(start_pos);
    float3 am = ambient_light * (1.0f - height_fraction) + (height_fraction * float3(1.0f, 1.0f, 1.0f));

    return light_color * transmittance * phase * scattering_coef;
}

//version 4
float4 do_cloud_ray_march(float3 cloud_layer_start, float3 cloud_layer_end, float3 to_light)
{
    // figure out the ray direction
    float3 ray_disp = cloud_layer_end - cloud_layer_start;
    float3 ray_dir = normalize(ray_disp);

    // determine number of steps based on if we are looking up or towards the horizon
    float horizontalness = 1.0f - abs(dot(ray_dir, float3(0.0f, 1.0f, 0.0f)));
    //uint num_steps = (uint) lerp(64.f, 128.f, horizontalness);
    uint num_steps = (uint)(MIDI.SLIDER_5 * 128.0f); // override with midi for testing

    // determine step vector and step_size
    float3 ds = ray_disp / (float) num_steps;
    float step_size = length(ds);

    // angle is used phase function
	float cos_angle = dot(normalize(to_light), normalize(ray_dir));

	// tracking variables
    float transmittance = 1.0f;
    float3 color = 0.0f;
    float opacity = 0.0f;

	// volumetric variables
	float3 scattering_coef = float3(0.5, 0.7, 1.0);
    scattering_coef.x = MIDI.SLIDER_0 * 2.0f;
    scattering_coef.y = MIDI.SLIDER_1 * 2.0f;
    scattering_coef.z = MIDI.SLIDER_2 * 2.0f;
	float absorption_coef = MIDI.SLIDER_4;
    float absorption_mult = -absorption_coef * step_size;

    float3 pos = cloud_layer_start;
    for (uint i = 0; i < num_steps; ++i)
    {
        // sample density at current location
        float mip = MIDI.KNOB_1 * 10.0f;
        float density = sample_density(pos, mip, false);
		density = saturate(density);

        // calculate transmittance at this pos and multiply that into current overall transmittance
        float dt = exp(absorption_mult * density);
        transmittance *= dt;

        float3 incident_light = calc_incident_light(pos, SUN_POSITION, cos_angle, absorption_coef, scattering_coef);

        float3 dc = transmittance * incident_light;
        color = color * (1.0f - transmittance) + dc;

        opacity += (1.0f - opacity) * (1.0f - dt);

        if(transmittance <= 0.00001f)
        {
            break;
        }

		// step to next sample location
        pos += ds;
    }

    color = pow(abs(color), 1.0 / 2.2); // gamma correction

    color *= opacity; //premultiplied alpha

    return float4(color, opacity);
}


//----------------------------------------------------------
// Main
//----------------------------------------------------------


float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	// get ray direction
	float2 ndc = get_ndc(data.uv);
	float3 ray_dir = get_ray_dir(ndc, FOCAL_LENGTH, CAMERA_TRANSFORM);

	// get camera world pos
    float3 eye_world_pos = get_eye_pos();
	float3 light = normalize(SUN_POSITION - eye_world_pos);

    // what we really want is the intersection points with the spheres that define the cloud layer
    float3 cloud_layer_start = 0.0f;
    float3 cloud_layer_end = 0.0f;
    bool got_cloud_layer = get_cloud_layer(eye_world_pos, ray_dir, cloud_layer_start, cloud_layer_end);

    if(!got_cloud_layer)
    {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    /*/
    // look at coverage only
    else
    {
        float cov = get_coverage(cloud_layer_start.xz);
        return float4(cov, cov, cov, cov);
    }
    /**/

    return do_cloud_ray_march(cloud_layer_start, cloud_layer_end, light);
}