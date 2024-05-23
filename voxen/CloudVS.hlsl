cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
}

cbuffer CloudConstantBuffer : register(b1)
{
    matrix world;
    float3 color;
    float density;
}

struct vsInput
{
    float3 position : POSITION;
    uint face : FACE;
};

struct vsOutput
{
    float4 posProj : SV_Position;
    float3 posWorld : POSITION;
    uint face : FACE;
};

vsOutput main(vsInput input)
{
    vsOutput output;
    
    output.face = input.face;
    
    output.posWorld = mul(float4(input.position, 1.0), world);
    
    output.posProj = mul(float4(output.posWorld, 1.0), view);
    output.posProj = mul(output.posProj, proj);
    
    return output;
}