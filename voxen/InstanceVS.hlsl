cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
}

struct vsInput
{
    float3 posModel : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    matrix instanceWorld : WORLD;
    uint type : TYPE;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

vsOutput main(vsInput input)
{
    vsOutput output;
    
    
    output.posProj = mul(float4(input.posModel, 1.0), input.instanceWorld);
    
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    
    // instance 자체의 world 고려 X
    // normal의 invTranspose 고려 X
    output.normal = input.normal;
    output.texcoord = input.texcoord;
    output.type = input.type;
    
    return output;
}