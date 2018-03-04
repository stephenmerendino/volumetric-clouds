struct vertex_in_t
{
	float3 position : POSITION;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	out_data.position = float4(vertex.position, 1.0f);

	return out_data;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	return float4(1, 0, 0, 1);
}
