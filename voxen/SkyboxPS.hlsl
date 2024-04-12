SamplerState g_linear : register(s0);
TextureCube g_cube : register(t0);

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

float4 main(vsOutput input) : SV_TARGET
{
    float3 normal = normalize(input.posWorld);
    //return g_cube.Sample(g_linear, posWorld);
    
    float3 mix = lerp(float3(0.2, 0.5, 0.8), float3(1.0, 1.0, 1.0), normal.y);
    return float4(mix, 1.0);

}