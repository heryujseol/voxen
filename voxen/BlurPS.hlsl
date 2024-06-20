Texture2D mirrorWorldTex : register(t0);

SamplerState linearClampSS : register(s2);

cbuffer postEffectConstantBuffer : register(b2)
{
    float dx;
    float dy;
    float strength;
    float threshold;
}

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

static const float gaussianKernel[5] = { 0.0545, 0.2442, 0.4026, 0.2442, 0.0545 };

float4 main(vsOutput input) : SV_TARGET
{
    float3 color = float3(0.0, 0.0, 0.0);
    float2 offset = float2(0.0, 0.0);
    float totalAlpha = 0.0;
    
    [unroll]
    for (int i = 0; i < 5; ++i)
    {
#ifdef BLUR_X
        offset = float2(dx * (i - 2), 0.0);
#endif
#ifdef BLUR_Y
        offset = float2(0.0, dy * (i - 2));
#endif
        float4 sampleColor = mirrorWorldTex.Sample(linearClampSS, input.texcoord + offset);

        sampleColor.rgb *= sampleColor.a;
        
        color += sampleColor.rgb * gaussianKernel[i];
        totalAlpha += sampleColor.a * gaussianKernel[i];
    }
    
    if (totalAlpha > 0.0)
    {
        color /= totalAlpha;
    }
    
    return float4(color, totalAlpha);
}