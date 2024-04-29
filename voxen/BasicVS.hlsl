
cbuffer ConstantBuffer : register(b0)
{
    matrix world;
}

cbuffer ConstantBuffer : register(b1)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy;
}

struct vsInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
};

vsOutput main(vsInput input, uint vID: SV_VertexID)
{
    vsOutput output;
    
    output.posWorld = mul(float4(input.pos, 1.0), world).xyz;
  
    output.posProj = float4(output.posWorld, 1.0);
    output.posProj = mul(output.posProj, view); 
    output.posProj = mul(output.posProj, proj);
    
    output.normalWorld = input.normal;
    
    uint n = vID % 4;
    if (n == 0)
        output.texcoord = float2(0.0, 0.0);
    else if (n == 1)
        output.texcoord = float2(1.0, 0.0);
    else if (n == 2)
        output.texcoord = float2(1.0, 1.0);
    else 
        output.texcoord = float2(0.0, 1.0);
    return output;
}