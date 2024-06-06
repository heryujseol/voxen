Texture2D atlasTexture : register(t0);
Texture2D grassColorMap : register(t1);

SamplerState pointClampSS : register(s0);

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
    float3 posWorld : POSITION;
    uint face : FACE;
    uint type : TYPE;
};

float2 getVoxelTexcoord(float3 pos, uint face)
{
    if (face == 0) // left
    {
        return float2(abs(pos.z - ceil(pos.z)), abs(pos.y - ceil(pos.y)));
    }
    else if (face == 1) // right
    {
        return float2(abs(pos.z - floor(pos.z)), abs(pos.y - ceil(pos.y)));
    }
    else if (face == 2) // bottom
    {
        return float2(abs(pos.x - floor(pos.x)), abs(pos.z - floor(pos.z)));
    }
    else if (face == 3) // top
    {
        return float2(abs(pos.x - floor(pos.x)), abs(pos.z - ceil(pos.z)));
    }
    else if (face == 4) // front
    {
        return float2(abs(pos.x - floor(pos.x)), abs(pos.y - ceil(pos.y)));
    }
    else // back
    {
        return float2(abs(pos.x - ceil(pos.x)), abs(pos.y - ceil(pos.y)));
    }   
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
    
    
    // atlas test
    // 2048 2048 -> 텍스쳐당 128x128, 그게 16x16
    float2 texcoord = getVoxelTexcoord(input.posWorld, input.face);
    uint texCount = 16;  // 한 줄의 텍스쳐 개수
    
    // [type * 6 + side] => 1차원 인덱스를 2차원 인덱스 좌표로 변경
    uint index = (input.type - 1) * 6 + input.face;

    uint2 indexUV = uint2(index % texCount, index / texCount);
    texcoord += indexUV; // x.u  y.v 
    texcoord /= texCount;
    
    float4 color = atlasTexture.Sample(pointClampSS, texcoord) * 0.3;
    
    float3 moonDir = -sunDir;
    float3 normal = getNormal(input.face);
    
    float ndotl = max(dot(sunDir, normal), 0.0);
    //if (sunDir.y < 0.0)
    //    ndotl = max(dot(moonDir, normal), 0.0);
    
    float strength = sunStrength;
    //if (sunDir.y < 0.0)
    //    strength = moonStrength * 0.5;
    
    
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
    
    //if (13700 <= dateTime && dateTime < 22300)
    //    color = color * (strength + 1.0);
    //else
    //    color = color * (strength + 1.0) * (ndotl + 1.0);
    
    return color;
}