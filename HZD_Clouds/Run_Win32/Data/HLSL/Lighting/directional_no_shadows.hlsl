Texture2D <float4> tDiffuse : register(t0);
Texture2D <float4> tNormal : register(t1);
Texture2D <float4> tSpec : register(t2);

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
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float3 worldPosition : WORLD_POSITION;
    float3 shadowPosition : SHADOW_POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
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
	out_data.tangent = mul(float4(vertex.tangent, 0.0), MODEL).xyz;
	out_data.bitangent = mul(float4(vertex.bitangent, 0.0), MODEL).xyz;

	return out_data;
}

float3 ColorNormalToSurfaceNormal(float3 colorNormal)
{
    float3 tbnNormal = (colorNormal * float3(2.0, 2.0, 1.0)) - float3(1.0, 1.0, 0.0);
    tbnNormal = normalize(tbnNormal);
    return tbnNormal;
}

float4 NormalToColor(float3 normal)
{
    return float4((normal + float3(1.0, 1.0, 1.0)) * 0.5, 1.0);
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    float4 finalColor;
    float4 specColor = 0.0;

	float4 textureColor = tDiffuse.Sample(sSampler, data.texCoord);
	float4 diffuse = textureColor * data.tint;

	float3 normal = normalize(data.normal);

    float3 faceNormal = normalize(data.normal);
    float3 tangent = normalize(data.tangent);
    float3 bitangent = normalize(data.bitangent);;

    float3x3 tbn = float3x3(tangent, bitangent, faceNormal);

    float3 colorNormal = tNormal.Sample(sSampler, data.texCoord).rgb;
    float3 surfaceNormal = ColorNormalToSurfaceNormal(colorNormal);
    surfaceNormal = mul(surfaceNormal, tbn);

    // Ambient
	float4 ambientFactor = float4(AMBIENT_LIGHT.rgb * AMBIENT_LIGHT.w, 1.0);

    // Directional
	float3 directionToLight = normalize(-DIRECTIONAL_LIGHTS[0].direction.xyz);
	float4 lightColor = float4(DIRECTIONAL_LIGHTS[0].color.rgb * DIRECTIONAL_LIGHTS[0].color.w, 1.0);
	float dot3 = saturate(dot(surfaceNormal, directionToLight));
	float4 directionalLightFactor = lightColor * dot3;

    // Add ambient and directional
    float4 diffuseFactor = saturate(ambientFactor + directionalLightFactor);

	finalColor = diffuse * diffuseFactor;
	return finalColor;
}