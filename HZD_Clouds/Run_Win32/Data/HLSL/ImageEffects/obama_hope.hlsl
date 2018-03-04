Texture2D <float4> tImage : register(t0);
SamplerState sSampler : register(s0);

struct vertex_in_t
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	out_data.position = float4(vertex.position, 1.0f);
	out_data.texCoord = vertex.texCoord;

	return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float4 diffuse = tImage.Sample(sSampler, data.texCoord);
	float grey = dot(diffuse.rgb, float3(0.3, 0.59, 0.11));

	float outputColor;

	float4 lightBlue = float4(0.5215, 0.5882, 0.6235, 1.0);
	float4 darkBlue = float4(0.0078, 0.1882, 0.3294, 1.0);
	float4 cream = float4(0.9882, 0.8941, 0.6588, 1.0);
	float4 red = float4(0.8471, 0.09803, 0.1294, 1.0);

	if (grey < 0.3f){
		return darkBlue;
	}
	else if (grey < 0.6f){
		return lightBlue;
	}
	else if (grey < 0.75f){
		return cream;
	}
	else{
		return red;
	}
}