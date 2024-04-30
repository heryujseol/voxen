SamplerState pointClampSS : register(s0);

Texture2D atlasTexture : register(t0);
Texture2D biomeColorMap : register(t1);
Texture2D topTexture : register(t2);
Texture2D sideTexture : register(t3);
Texture2D dirtTexture : register(t4);

cbuffer ConstantBuffer : register(b1)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy;
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
};

float2 getVoxelTexcoord(float3 pos, float3 normal)
{
    if (normal.z < 0.0) // front
    {
        return float2(abs(pos.x - floor(pos.x)), abs(pos.y - ceil(pos.y)));
    }
    else if (normal.z > 0.0) // back
    {
        return float2(abs(pos.x - ceil(pos.x)), abs(pos.y - ceil(pos.y)));
    }
    else if (normal.y > 0.0) // top
    {
        return float2(abs(pos.x - floor(pos.x)), abs(pos.z - ceil(pos.z)));
    }
    else if (normal.y < 0.0) // bottom
    {
        return float2(abs(pos.x - floor(pos.x)), abs(pos.z - floor(pos.z)));
    }
    else if (normal.x < 0.0) // left
    {
        return float2(abs(pos.z - ceil(pos.z)), abs(pos.y - ceil(pos.y)));
    }
    else // right
    {
        return float2(abs(pos.z - floor(pos.z)), abs(pos.y - ceil(pos.y)));
    }
}

uint get_side(float3 normal) // 임시 함수
{
    if (normal.z < 0.0) // front
    {
        return 0;
    }
    else if (normal.z > 0.0) // back
    {
        return 1;
    }
    else if (normal.y > 0.0) // top
    {
        return 2;
    }
    else if (normal.y < 0.0) // bottom
    {
        return 3;
    }
    else if (normal.x < 0.0) // left
    {
        return 4;
    }
    else // right
    {
        return 5;
    }
}

float4 main(vsOutput input) : SV_TARGET
{
    float2 texcoord = getVoxelTexcoord(input.posWorld, input.normalWorld);
    
    float temperature = 0.5;
    float downfall = 1.0;
    float4 biome = biomeColorMap.SampleLevel(pointClampSS, float2(1 - temperature, 1 - temperature / downfall), 0.0);
    /*
    float4 color;
    if (input.normalWorld.y > 0.0) // top
        color = topTexture.Sample(pointClampSS, texcoord) * biome;
    else
    {
        color = dirtTexture.Sample(pointClampSS, texcoord);
        
        float4 sideOverlay = sideTexture.SampleLevel(pointClampSS, texcoord, 0.0);
        if (sideOverlay.r > 0.0)
        {
            color = sideOverlay * biome;
        }
    }*/
    
    // atlas test
    // 2048 2048 -> 텍스쳐당 128x128, 그게 16x16
    uint tex_count = 16;  // 한 줄의 텍스쳐 개수
    
    // [type * 6 + side] => 1차원 인덱스를 2차원 인덱스 좌표로 변경
    uint type = 0; // 0 ~ 255
    uint side = get_side(input.normalWorld);
    uint index = type * 6 + side;

    uint2 index_uv = uint2(index % tex_count, index / tex_count);
    texcoord += index_uv; // x.u  y.v 
    texcoord /= tex_count;
    
    float4 color = atlasTexture.Sample(pointClampSS, texcoord);
    
    return color;
}