TextureCube envMapTexture : register(t0);

SamplerState linearClampSS : register(s2);

cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyePos;
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
};

float3 getNormal(uint face)
{
    if (face == 0)
    {
        return float3(-1.0, 0.0, 0.0);
    }
    else if (face == 1)
    {
        return float3(1.0, 0.0, 0.0);
    }
    else if (face == 2)
    {
        return float3(0.0, -1.0, 0.0);
    }
    else if (face == 3)
    {
        return float3(0.0, 1.0, 0.0);
    }
    else if (face == 4)
    {
        return float3(0.0, 0.0, -1.0);
    }
    else
    {
        return float3(0.0, 0.0, 1.0);
    }
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 normal = getNormal(input.face);
    float3 toEye = normalize(eyePos - input.posWorld);
    
    if (normal.y <= 0 || input.posWorld.y != 62)
        discard;
    
    float3 color = envMapTexture.Sample(linearClampSS, reflect(-toEye, normal)).rgb;
    
    return float4(color, 1.0);
}