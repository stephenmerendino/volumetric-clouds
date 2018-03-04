Texture2D <float4> tImage : register(t0);
SamplerState sSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	float4x4 MODEL;
	float4x4 VIEW;
	float4x4 PROJECTION;
};

cbuffer TimeBuffer : register(b1)
{
	float GAME_TIME;
	float SYSTEM_TIME;
	float GAME_FRAME_TIME;
	float SYSTEM_FRAME_TIME;
};

struct DirectionalLight 
{
	float4 direction;
	float4 color;
};

struct PointLight
{
	float4 position;
	float4 color;
	float4 attenuation;
    float4 specAttenuation;
};

struct SpecularInfo
{
    float power;
    float factor;
    float padding[2];
};

cbuffer LightBuffer : register(b2)
{
	float4				EYE_WORLD_POSITION;
	float4				AMBIENT_LIGHT; // <r, g, b, intensity>
	DirectionalLight	DIRECTIONAL_LIGHTS[4];
	PointLight			POINT_LIGHTS[16];
    SpecularInfo        SPECULAR;
};

struct vertex_in_t
{
	float3 position : POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	float4 worldSpaceVertex = mul(float4(vertex.position, 1.0f), MODEL);
	float4 cameraSpaceVertex = mul(float4(worldSpaceVertex), VIEW);
	float4 clipSpaceVertex = mul(float4(cameraSpaceVertex), PROJECTION);

	out_data.position = clipSpaceVertex;
	out_data.tint = vertex.tint;
	out_data.texCoord = vertex.texCoord;
	out_data.normal = mul(float4(vertex.normal, 0.0), MODEL).xyz;

	return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float4 textureDiffuse = tImage.Sample(sSampler, data.texCoord);

	float3 normal = normalize(data.normal);

	float4 ambientLightFactor = float4(AMBIENT_LIGHT.rgb * AMBIENT_LIGHT.w, 1.0);

    float4 directionalLightFactors = 0.0f;
    for (int dirLightIndex = 0; dirLightIndex < 4; ++dirLightIndex)
    {
        float4 directionalLightColor = float4(DIRECTIONAL_LIGHTS[dirLightIndex].color.rgb * DIRECTIONAL_LIGHTS[dirLightIndex].color.w, 1.0);
    	float3 directionNormalized = normalize(DIRECTIONAL_LIGHTS[dirLightIndex].direction.xyz);
    	float directionalLightFactor = saturate(dot(-directionNormalized, normal));

        directionalLightFactors += directionalLightFactor * directionalLightColor;
    }

	float4 diffuseLightFactor = ambientLightFactor + directionalLightFactors;
	diffuseLightFactor = saturate(diffuseLightFactor);

	float4 diffuseColor = textureDiffuse * diffuseLightFactor;
	return diffuseColor;
}