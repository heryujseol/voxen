cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
}

struct vsInput
{
    float3 position : POSITION;
    uint face : FACE;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

vsOutput main(vsInput input)
{
    vsOutput output;
    
    output.posWorld = input.position;
    
    output.posProj = mul(float4(output.posWorld, 0.0), view);
    output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);
    
    return output;
}