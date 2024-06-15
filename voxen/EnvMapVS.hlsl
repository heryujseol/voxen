struct vsInput
{
    float3 position : POSITION;
};

struct vsOutput
{
    float4 position : SV_Position;
};

vsOutput main(vsInput vsInput)
{
    vsOutput output;
    
    output.position = float4(vsInput.position, 1.0);
    
    return output;
}