SamplerState g_linear : register(s0);
TextureCube g_cube : register(t0);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

float4 main(vsOutput input) : SV_TARGET
{
    float3 normalizedPosition = normalize(input.posWorld);
    
    return g_cube.Sample(g_linear, normalizedPosition);
}