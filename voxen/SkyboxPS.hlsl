struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

float4 main(vsOutput input) : SV_TARGET
{
    float3 dir = normalize(input.posWorld);
    
    float3 color = lerp(float3(0.2, 0.5, 0.8), float3(1.0, 1.0, 1.0), dir.y);
    return float4(color, 1.0);

}