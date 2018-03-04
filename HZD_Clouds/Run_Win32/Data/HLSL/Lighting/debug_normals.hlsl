Texture2D <float4> tImage : register(t0);
Texture2D <float4> tNormalMap : register(t1);
Texture2D <float4> tSpecMap : register(t2);

SamplerState sLinearSampler : register(s0);
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
	float4				EYE_WORLD_POSITION;
	float4				AMBIENT_LIGHT;
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
    float4 tangent : TANGENT;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float3 worldPosition : WORLD_POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
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
	out_data.worldPosition = worldSpaceVertex.xyz;
	out_data.tint = vertex.tint;
	out_data.texCoord = vertex.texCoord;

    out_data.normal = mul(float4(vertex.normal, 0.0f), MODEL).xyz;
    out_data.tangent = mul(float4(vertex.tangent.xyz, 0.0f), MODEL);
    out_data.tangent.w = vertex.tangent.w;

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
    return float4(normal + float3(1.0, 1.0, 1.0) * 0.5, 1.0);
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    float4 finalColor;
    float4 specColor = 0.0;

	float4 textureColor = tImage.Sample(sLinearSampler, data.texCoord);
	float4 diffuse = textureColor * data.tint;

    float3 fragmentToEye = EYE_WORLD_POSITION.xyz - data.worldPosition;
    float3 eyeToFragment = -normalize(fragmentToEye);

	float3 faceNormal = normalize(data.normal);
    float3 tangent = normalize(data.tangent.xyz);
    float3 binormal = cross(faceNormal, tangent) * data.tangent.w;

    float3x3 tbn = float3x3(tangent, binormal, faceNormal);

    float3 colorNormal = tNormalMap.Sample(sLinearSampler, data.texCoord).rgb;
    float3 surfaceNormal = ColorNormalToSurfaceNormal(colorNormal);
    surfaceNormal = mul(surfaceNormal, tbn);

    return NormalToColor(surfaceNormal);

    // Ambient
	float4 ambientFactor = float4(AMBIENT_LIGHT.rgb * AMBIENT_LIGHT.w, 1.0);

    // Point lights
	float4 pointLightFactors = 0.0f;
	for (int index = 0; index < 16; ++index){
        PointLight pointLight = POINT_LIGHTS[index];

    	float3 vectorToLight = pointLight.position.xyz - data.worldPosition;
    	float distanceToLight = length(vectorToLight);

    	float3 directionToLight = vectorToLight / distanceToLight;

		float4 lightColor = float4(pointLight.color.rgb, 1.0);
        float lightIntensity = pointLight.color.w;

        // Diffuse light
		float dot3 = saturate(dot(surfaceNormal, directionToLight));

        // Diffuse light attenuation
		float3 lightAttenuation = pointLight.attenuation.xyz;
		float attenuationFactor = lightIntensity / 
								 (lightAttenuation.x + 
								  lightAttenuation.y * distanceToLight + 
								  lightAttenuation.z * distanceToLight * distanceToLight);

		attenuationFactor = saturate(attenuationFactor);

		float4 diffuseLightColor = lightColor * dot3 * attenuationFactor;
        pointLightFactors += diffuseLightColor;

        // Spec light
        float3 specAttenuation = pointLight.specAttenuation.xyz;
        float specAttenuationFactor = lightIntensity /
								 (specAttenuation.x + 
								  specAttenuation.y * distanceToLight + 
								  specAttenuation.z * distanceToLight * distanceToLight);

        specAttenuationFactor = saturate(specAttenuationFactor);

        float3 lightDirReflected = reflect(directionToLight, surfaceNormal);
        float specDot3 = saturate(dot(lightDirReflected, eyeToFragment));

        float4 colorFromSpecMap = tSpecMap.Sample(sLinearSampler, data.texCoord);
        float specFactor = specAttenuationFactor * colorFromSpecMap.x * pow(specDot3, SPECULAR.power);
        specColor += (specFactor * lightColor);
    }

	float4 diffuseFactor = saturate(ambientFactor + pointLightFactors);

    finalColor = (diffuse * diffuseFactor) + specColor;
    return finalColor;
}
