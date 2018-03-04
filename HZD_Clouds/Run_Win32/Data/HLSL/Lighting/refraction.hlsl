TextureCube <float4> tReflectionMap : register(t0);

SamplerState sLinearSampler : register(s0);

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
	float4				AMBIENT_LIGHT;
	DirectionalLight	DIRECTIONAL_LIGHTS[4];
	PointLight			POINT_LIGHTS[16];
    SpecularInfo        SPECULAR;
};

cbuffer RefractionBuffer : register(b3)
{
    float REFRACTION_RATIO;
    float3 PADDING;
}

struct vertex_in_t
{
	float3 position : POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
    float3 worldPosition : POSITION;
    float2 uv : UV;
	float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	float4 worldSpaceVertex = mul(float4(vertex.position, 1.0f), MODEL);
	float4 cameraSpaceVertex = mul(float4(worldSpaceVertex), VIEW);
	float4 clipSpaceVertex = mul(float4(cameraSpaceVertex), PROJECTION);

	out_data.position = clipSpaceVertex;
    out_data.worldPosition = vertex.position;
    out_data.uv = vertex.texCoord;
	out_data.normal = mul(float4(vertex.normal, 0.0f), MODEL).xyz;

	return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    float3 fragmentToEye = EYE_WORLD_POSITION.xyz - data.worldPosition;
    float3 eyeToFragment = -normalize(fragmentToEye);

    float3 normal = normalize(data.normal);

    float3 eyeRefract = refract(eyeToFragment, normal, REFRACTION_RATIO);

    return tReflectionMap.Sample(sLinearSampler, eyeRefract);
}
