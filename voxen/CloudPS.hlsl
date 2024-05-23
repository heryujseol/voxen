struct vsOutput
{
    float4 posProj : SV_Position;
    float3 posWorld : POSITION;
    uint face : FACE;
};

float3 getFaceColor(uint face)
{
    if (face == 0 || face == 1)
    {
        return float3(0.86, 0.86, 0.86);
    }
    else if (face == 4 || face == 5)
    {
        return float3(0.93, 0.93, 0.93);
    }
    else if (face == 3)
    {
        return float3(1.0, 1.0, 1.0);
    }
    else // 2
    {
        return float3(0.7, 0.7, 0.7);
    }
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 color = float3(0.0, 0.0, 0.0);
    
    
    return float4(getFaceColor(input.face), 0.8);
}