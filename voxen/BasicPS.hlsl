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

float4 main(vsOutput input) : SV_TARGET
{
    /*
    float temperature = 0.5;
    float downfall = 1.0;
    float4 biome = biomeColorMap.SampleLevel(pointClampSS, float2(1 - temperature, 1 - temperature / downfall), 0.0);
    */
    
    // atlas test
    // 2048 2048 -> 텍스쳐당 128x128, 그게 16x16
    float2 texcoord = getVoxelTexcoord(input.posWorld, input.face);
    uint tex_count = 16;  // 한 줄의 텍스쳐 개수
    
    // [type * 6 + side] => 1차원 인덱스를 2차원 인덱스 좌표로 변경
    uint index = input.type * 6 + input.face;

    uint2 index_uv = uint2(index % tex_count, index / tex_count);
    texcoord += index_uv; // x.u  y.v 
    texcoord /= tex_count;
    
    float4 color = atlasTexture.Sample(pointClampSS, texcoord);
    
    return color;
}