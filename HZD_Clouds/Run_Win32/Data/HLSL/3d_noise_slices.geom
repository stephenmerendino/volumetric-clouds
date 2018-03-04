struct geometry_in_t
{
	float4 position : SV_Position;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 bone_weights : BONE_WEIGHTS;
    uint4 bone_indices : BONE_INDICES;
};

struct geometry_out_t
{
    float4 position : SV_Position;
    uint layer : SV_RenderTargetArrayIndex;
};

cbuffer noise_gen_parameters_buffer : register(b4)
{
    unsigned int START_SLICE;
    float3 _PADDING;
};

[maxvertexcount(96)]
void GeometryFunction(triangle geometry_in_t input_stream[3], inout TriangleStream<geometry_out_t> out_stream)
{
    geometry_out_t output0 = (geometry_out_t)0;
    geometry_out_t output1 = (geometry_out_t)0;
    geometry_out_t output2 = (geometry_out_t)0;

    output0.position = input_stream[0].position;
    output1.position = input_stream[1].position;
    output2.position = input_stream[2].position;

    for(unsigned int i = START_SLICE; i < START_SLICE + 32; i++){
        output0.layer = i;
        output1.layer = i;
        output2.layer = i;

        out_stream.Append(output0);
        out_stream.Append(output1);
        out_stream.Append(output2);

        out_stream.RestartStrip();
    }
};
