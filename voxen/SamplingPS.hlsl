Texture2D samplingTexture : register(t0);

SamplerState linearWrapSS : register(s1);

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(vsOutput input) : SV_TARGET
{
    return samplingTexture.SampleLevel(linearWrapSS, input.texcoord, 0.0);
}