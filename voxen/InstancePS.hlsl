SamplerState pointWrapSS : register(s0);
Texture2DArray atlasTextureArray : register(t0);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 normal : NORMAL;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

float4 main(vsOutput input) : SV_TARGET
{
    float4 color = atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, (float)input.type), 0.0);
    if (color.a != 1.0)
    {
        discard;
    }
        
    return color;
}