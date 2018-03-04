Texture2D <float4> tImage : register(t0);
SamplerState sSampler : register(s0);

struct vertex_in_t
{
	float3 position : POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	out_data.position = float4(vertex.position, 1.0f);
	out_data.tint = vertex.tint;
	out_data.texCoord = vertex.texCoord;

	return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float4 diffuse = tImage.Sample(sSampler, data.texCoord);

	float3x3 sepia_transform = float3x3( 
	  float3( 0.393f, 0.349f, 0.272f ),
	  float3( 0.769f, 0.686f, 0.534f ),
	  float3( 0.189f, 0.168f, 0.131f )
	);
	float3 sepia = mul( diffuse.xyz, sepia_transform );

	float4 final_color = float4( sepia, 1.0f );
	return final_color;
}