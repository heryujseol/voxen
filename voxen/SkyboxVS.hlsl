struct vsInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

cbuffer ConstantBuffer : register(b1)
{
    matrix view;
    matrix proj;
}

vsOutput main(vsInput input)
{
    vsOutput output;
    
    output.posWorld = input.pos;
    
    output.posProj = mul(float4(output.posWorld, 0.0), view);
    output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);
    
    return output;
}