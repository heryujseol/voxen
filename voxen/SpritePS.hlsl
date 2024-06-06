Texture2DArray atlasTextureArray : register(t0);

SamplerState pointWrapSS : register(s0);

struct gsOutput
{
    float4 posProj : SV_Position;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

float4 main(gsOutput input) : SV_TARGET
{
    float4 color = atlasTextureArray.SampleLevel(pointWrapSS, float3(input.texcoord, input.type), 0.0);
    
    if (color.a != 1.0)
        discard;
    return color;
}