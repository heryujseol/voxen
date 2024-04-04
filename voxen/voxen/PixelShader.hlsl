struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

float4 main(vsOutput input) : SV_TARGET
{
    return float4(input.color, 1.0);
}