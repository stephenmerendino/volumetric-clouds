Texture2D <float4> tImage : register(t0);
Texture2D tDepthMap : register(t1);
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
    float4 EYE_WORLD_POSITION;
    float4 AMBIENT_LIGHT; // <r, g, b, intensity>
    DirectionalLight DIRECTIONAL_LIGHTS[4];
    PointLight POINT_LIGHTS[16];
    SpecularInfo SPECULAR;
};

cbuffer LightMatrixBuffer : register(b3)
{
	float4x4 LIGHT_MODEL;
	float4x4 LIGHT_VIEW;
	float4x4 LIGHT_PROJECTION;
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
	float3 worldPosition : WORLD_POSITION;
    float3 shadowPosition : SHADOW_POSITION;
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

    float4 shadowWorldSpaceVertex = mul(float4(vertex.position, 1.0f), LIGHT_MODEL);
    float4 shadowViewSpaceVertex = mul(shadowWorldSpaceVertex, LIGHT_VIEW);
    float4 shadowClipSpaceVertex = mul(shadowViewSpaceVertex, LIGHT_PROJECTION);

	out_data.position = clipSpaceVertex;
	out_data.worldPosition = worldSpaceVertex.xyz;
    out_data.shadowPosition = shadowClipSpaceVertex.xyz;
	out_data.tint = vertex.tint;
	out_data.texCoord = vertex.texCoord;
	out_data.normal = mul(float4(vertex.normal, 0.0), MODEL).xyz;

	return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float4 textureColor = tImage.Sample(sSampler, data.texCoord);
	float4 diffuse = textureColor * data.tint;

	float3 normal = normalize(data.normal);

    // Ambient
	float4 ambientFactor = float4(AMBIENT_LIGHT.rgb * AMBIENT_LIGHT.w, 1.0);

    // Directional
	float3 directionToLight = normalize(-DIRECTIONAL_LIGHTS[0].direction.xyz);
	float4 lightColor = float4(DIRECTIONAL_LIGHTS[0].color.rgb * DIRECTIONAL_LIGHTS[0].color.w, 1.0);
	float dot3 = saturate(dot(normal, directionToLight));
	float4 directionalLightFactor = lightColor * dot3;

    // Shadow
    float visibility = 1.0;

    float shadowU = data.shadowPosition.x * 0.5 + 0.5;
    float shadowV = data.shadowPosition.y * 0.5 + 0.5;
    shadowV = 1.0 - shadowV;
    float shadowMapDepth = tDepthMap.Sample(sSampler, float2(shadowU, shadowV)).r;
    float fragmentShadowDepth = data.shadowPosition.z;

    float bias = 0.005;
    if (shadowMapDepth < fragmentShadowDepth - bias)
    {
        visibility = 0.1f;
    }

    // Add ambient and directional
	float4 diffuseFactor = saturate(ambientFactor + (visibility * directionalLightFactor));

	float4 finalColor = diffuse * diffuseFactor;
	return finalColor;
}