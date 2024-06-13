cbuffer EnvMapConstantBuffer : register(b0)
{
    matrix view[6];
    matrix proj;
}

cbuffer ChunkConstantBuffer : register(b1)
{
    matrix world;
}

struct vsOutput
{
    float4 position : SV_Position;
    uint face : FACE;
    uint type : TYPE;
};

struct gsOutput
{
    float4 posProj : SV_POSITION;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
    float3 posWorld : POSITION1;
    float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
};

[maxvertexcount(18)]
void main(triangle vsOutput input[3] : SV_POSITION,
	inout TriangleStream<gsOutput> outputStream
)
{
    gsOutput output;
    output.face = input[0].face;
    output.type = input[0].type;
    
    for (uint face = 0; face < 6; ++face)
	{
        output.renderTargetArrayIndex = face;
        
        for (uint i = 0; i < 3; ++i)
        {
            output.posModel = input[i].position.xyz;
            
            output.posWorld = mul(input[i].position, world).xyz;
            
            output.posProj = mul(float4(output.posWorld, 1.0), view[face]);
            output.posProj = mul(output.posProj, proj);
            
            outputStream.Append(output);
        }
        
        outputStream.RestartStrip();
    }
}