SamplerState g_sampler : register(s0);

Texture2D g_side : register(t0);
Texture2D g_top : register(t1);
Texture2D g_dirt : register(t2);
Texture2D g_colorMap : register(t3);

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
    float4 biome = g_colorMap.Sample(g_sampler, float2(1 - temperature, 1 - temperature / downfall));
    float4 color;
    if (input.normalWorld.y > 0.0)
        color = g_top.Sample(g_sampler, input.texcoord) * biome;
    else
    {
        color = g_side.Sample(g_sampler, input.texcoord) * biome;
        if (color.a == 0.0)
            color += g_dirt.Sample(g_sampler, input.texcoord);
    }
        
        
    return color;
}