Texture2DArray atlasTextureArray : register(t0);
Texture2D grassColorMap : register(t1);

SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);
SamplerState linearClampSS : register(s2);

#ifdef USE_DEPTH_CLIP
    Texture2D depthTex : register(t2);
#endif

cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy;
}

cbuffer SkyboxConstantBuffer : register(b1)
{
    float3 sunDir;
    float skyScale;
    float3 normalHorizonColor;
    uint dateTime;
    float3 normalZenithColor;
    float sunStrength;
    float3 sunHorizonColor;
    float moonStrength;
    float3 sunZenithColor;
    float dummy3;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
};

float2 getVoxelTexcoord(float3 pos, uint face)
{
    float2 texcoord = float2(0.0, 0.0);
    
    if (face == 0) // left
    {
        texcoord = float2(-pos.z + 32.0, -pos.y + 32.0);
    }
    else if (face == 1) // right
    {
        texcoord = float2(pos.z, -pos.y + 32.0);
    }
    else if (face == 2) // bottom
    {
        texcoord = float2(pos.x, pos.z);
    }
    else if (face == 3) // top
    {
        texcoord = float2(pos.x, -pos.z + 32.0);
    }
    else if (face == 4) // front
    {
        texcoord = float2(pos.x, -pos.y + 32.0);
    }
    else // back
    {
        texcoord = float2(-pos.x + 32.0, -pos.y + 32.0);
    }

    return texcoord;
}

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
    //float temperature = 0.5;
    //float downfall = 1.0;
    //float4 biome = grassColorMap.SampleLevel(pointClampSS, float2(1 - temperature, 1 - temperature / downfall), 0.0);
    
    float2 texcoord = getVoxelTexcoord(input.posModel, input.face);
    uint index = (input.type - 1) * 6 + input.face;
    
#ifdef USE_ALPHA_CLIP 
    if (atlasTextureArray.SampleLevel(pointWrapSS, float3(texcoord, index), 0.0).a != 1.0)
        discard;
#endif
    
#ifdef USE_DEPTH_CLIP
    float width, height, lod;
    depthTex.GetDimensions(0, width, height, lod);
    
    float2 screenTexcoord = float2(input.posProj.x / width, input.posProj.y / height);
    float planeDepth = depthTex.Sample(linearClampSS, screenTexcoord).r;
    float pixelDepth = input.posProj.z;

    // Discard pixel if its depth is less than the depth buffer's value
    if (pixelDepth < planeDepth)
    {
        discard;
    }
#endif
    
    float3 color = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, index)).rgb * 0.3;
    
    float3 normal = getNormal(input.face);
    
    float ndotl = max(dot(sunDir, normal), 0.0);
    
    float strength = sunStrength;
    
    if (13700 <= dateTime && dateTime <= 14700)
    {
        float w = (dateTime - 13700) / 1000.0;
        color = lerp(color * (strength + 1.0) * (ndotl + 1.0), color, w);
    }
    else if (21300 <= dateTime && dateTime <= 22300)
    {
        float w = (dateTime - 21300) / 1000.0;
        color = lerp(color, color * (strength + 1.0) * (ndotl + 1.0), w);
    }
    else if (14700 < dateTime && dateTime < 21300)
    {
        color = color * (strength + 1.0);
    }
    else
    {
        color = color * (strength + 1.0) * (ndotl + 1.0);
    }

    return float4(color, 1.0);
}