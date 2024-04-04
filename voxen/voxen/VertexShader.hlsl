struct vsInput
{
    float3 pos : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix invTrans;
}

cbuffer ConstantBuffer : register(b1)
{
    matrix view;
    matrix proj;
}

vsOutput main(vsInput input)
{
    vsOutput output;
    
    output.posWorld = mul(float4(input.pos, 1.0), world).xyz;
    
    output.color = input.color;
    
    output.normal = mul(float4(input.normal, 0.0), invTrans).xyz;
    output.normal = normalize(output.normal);
    
    output.texcoord = input.texcoord;
    
    output.posProj = float4(output.posWorld, 1.0);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    
    return output;
}