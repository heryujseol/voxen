Texture2DArray atlasTextureArray : register(t0);
TextureCube envMapTexture : register(t1);

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

// fresnelR0는 물질의 고유 성질
// Water : (0.02, 0.02, 0.02)
// Glass : (0.08, 0.08, 0.08)
// Plastic : (0.05, 0.05, 0.05)
// Gold: (1.0, 0.71, 0.29)
// Silver: (0.95, 0.93, 0.88)
// Copper: (0.95, 0.64, 0.54)
float3 getReflectionCoefficient(uint type)
{
    float3 fresnel = float3(1.0, 1.0, 1.0);
    
    if (type == 1) // water
    {
        return float3(0.02, 0.02, 0.02);
    }
    
    return fresnel;
}

float3 schlickFresnel(float3 N, float3 E, float3 R)
{
    // https://en.wikipedia.org/wiki/Schlick%27s_approximation
    // [f0 ~ 1]
    // 90도 -> dot(N,E)==0 -> f0+(1-f0)*1^5 -> 1
    //  0도 -> dot(N,E)==1 -> f0+(1-f0)*0*5 -> f0
    return R + (1 - R) * pow((1 - max(dot(N, E), 0.0)), 5.0);
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 normal = getNormal(input.face);
    float3 toEye = normalize(eyePos - input.posWorld);
    
    if (normal.y <= 0)
        discard;
    
    float3 color = envMapTexture.Sample(linearClampSS, reflect(-toEye, normal)).rgb;
    
    return float4(color, 1.0);
}