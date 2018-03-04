Texture2D tDiffuse : register(t0);
Texture2D tNormal : register(t1);
Texture2D tDepth : register(t2);
SamplerState sSampler : register(s0);
SamplerState sPointSampler : register(s1);

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
    float2 texCoord : TEXCOORD;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
    vertex_to_fragment_t out_data = (vertex_to_fragment_t) 0;

    float4 worldSpaceVertex = mul(float4(vertex.position, 1.0f), MODEL);
    float4 cameraSpaceVertex = mul(float4(worldSpaceVertex), VIEW);
    float4 clipSpaceVertex = mul(float4(cameraSpaceVertex), PROJECTION);

    out_data.position = clipSpaceVertex;
    out_data.texCoord = vertex.texCoord;

    return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    // sample depth at uv
    // mul by inverse view proj to get world space
    // world multiplied by light cob to compare against depth

    float4 diffuse = tDiffuse.Sample(sSampler, data.texCoord);

    float3 normalAsColor = tNormal.Sample(sPointSampler, data.texCoord).rgb;
    float3 surfaceNormal = normalAsColor * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);

    float fragmentDepth = tDepth.Sample(sSampler, data.texCoord).r;

    // Ambient
    float4 ambientFactor = float4(AMBIENT_LIGHT.rgb * AMBIENT_LIGHT.w, 1.0);

    // Directional
    float3 directionToLight = normalize(-DIRECTIONAL_LIGHTS[0].direction.xyz);
    float4 lightColor = float4(DIRECTIONAL_LIGHTS[0].color.rgb * DIRECTIONAL_LIGHTS[0].color.w, 1.0);
    float dot3 = saturate(dot(surfaceNormal, directionToLight));
    float4 directionalLightFactor = lightColor * dot3;

    // Add ambient and directional
    float4 diffuseFactor = saturate(ambientFactor + directionalLightFactor);

    float4 finalColor = diffuse * diffuseFactor;
    return finalColor;
}