cbuffer EnvMapConstantBuffer : register(b0)
{
    matrix view[6];
    matrix proj;
}

struct vsOutput
{
    float4 position : SV_Position;
};

struct gsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void main(triangle vsOutput input[3] : SV_POSITION,
	inout TriangleStream<gsOutput> outputStream
)
{
    gsOutput output;
    
    for (uint face = 0; face < 6; ++face)
    {
        output.renderTargetArrayIndex = face;
        
        for (uint i = 0; i < 3; ++i)
        {
            output.posWorld = input[i].position.xyz;
            
            output.posProj = mul(float4(output.posWorld, 0.0), view[face]);
            output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);
            
            outputStream.Append(output);
        }
        
        outputStream.RestartStrip();
    }
}