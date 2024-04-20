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

float4 main(vsOutput input) : SV_TARGET
{
    float temperature = 0.5;
    float downfall = 1.0;
    float4 biome = biomeColorMap.SampleLevel(pointClampSS, float2(1 - temperature, 1 - temperature / downfall), 0.0);
    
    //float4 color = atlasTexture.Sample(pointClampSS, input.texcoord) * biome;
    
    float4 color;
    if (input.normalWorld.y > 0.0)
        color = topTexture.Sample(pointClampSS, input.texcoord) * biome;
    else
    {
        color = dirtTexture.Sample(pointClampSS, input.texcoord);
        
        float4 sideOverlay = sideTexture.SampleLevel(pointClampSS, input.texcoord, 0.0);
        if (sideOverlay.r > 0.0)
        {
            color = sideOverlay * biome;
        }
    }
    
    return color;
}