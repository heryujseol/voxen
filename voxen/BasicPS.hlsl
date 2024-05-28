SamplerState pointClampSS : register(s0);

Texture2D atlasTexture : register(t0);
Texture2D grassColorMap : register(t1);

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
    float3 dir = sunDir;
    float3 normal = getNormal(input.face);
    
    float ndotl = max(dot(dir, normal), 0.0);
    
    float2 texcoord = getVoxelTexcoord(input.posWorld, input.face);
    uint texCount = 16;  // 한 줄의 텍스쳐 개수
    
    // [type * 6 + side] => 1차원 인덱스를 2차원 인덱스 좌표로 변경
    uint index = (input.type - 1) * 6 + input.face;

    uint2 indexUV = uint2(index % texCount, index / texCount);
    texcoord += indexUV; // x.u  y.v 
    texcoord /= texCount;
    
    float4 color = atlasTexture.Sample(pointClampSS, texcoord);
    color *= 0.8;
    
    if (ndotl == 0)
    {
        color = color;
    }
    else
    {
        color *= ndotl + 1.0f;
    }
    float2 ddX = ddx(texcoord);
    float2 ddY = ddy(texcoord);
    
    //float4 color = atlasTexture.SampleGrad(pointClampSS, texcoord, ddX, ddY);
    float4 color = atlasTexture.SampleLevel(pointClampSS, texcoord, 5.0);
    //float4 color = atlasTexture.Sample(pointClampSS, texcoord);
    
    //if (input.type == 2 && input.face == 3)
    //    color = biome * color;
    
    //float4 color = tex2Dgrad(atlasTexture, texcoord, ddX, ddY);
    
    return color;
}