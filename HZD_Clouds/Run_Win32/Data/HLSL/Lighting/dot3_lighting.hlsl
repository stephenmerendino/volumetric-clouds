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
    float4 EYE_WORLD_POSITION;
    float4 AMBIENT_LIGHT; // <r, g, b, intensity>
    DirectionalLight DIRECTIONAL_LIGHTS[4];
    PointLight POINT_LIGHTS[16];
    SpecularInfo SPECULAR;
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
	out_data.worldPosition = worldSpaceVertex.xyz;
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

	float4 ambientFactor = float4(AMBIENT_LIGHT.rgb * AMBIENT_LIGHT.w, 1.0);
	float4 pointLightFactors = 0.0f;

	for (int index = 0; index < 16; ++index){
		float3 vectorToLight = POINT_LIGHTS[index].position.xyz - data.worldPosition;
		float distanceToLight = length(vectorToLight);
		float3 directionToLight = vectorToLight / distanceToLight;

    	float4 lightColor = float4(POINT_LIGHTS[index].color.rgb, 1.0);
        float lightIntensity = POINT_LIGHTS[index].color.w;

		float dot3 = saturate(dot(normal, directionToLight));

		// attenuation
		float3 lightAttenuation = POINT_LIGHTS[index].attenuation.xyz;
		float attenuationFactor = lightIntensity / 
								 (lightAttenuation.x + 
								  lightAttenuation.y * distanceToLight + 
								  lightAttenuation.z * distanceToLight * distanceToLight);

		attenuationFactor = saturate(attenuationFactor);

		pointLightFactors += lightColor * dot3 * attenuationFactor;
	}

	float4 diffuseFactor = saturate(ambientFactor + pointLightFactors);

	float4 finalColor = diffuse * diffuseFactor;
	return finalColor;
}