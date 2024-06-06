struct vsOutput
{
    float4 posModel : SV_Position;
    uint type : TYPE;
};

vsOutput main(uint data : DATA)
{
    int x = (data >> 23) & 63;
    int y = (data >> 17) & 63;
    int z = (data >> 11) & 63;
    uint type = data & 255;
    
    vsOutput output;
    output.posModel = float4(float(x), float(y), float(z), 1.0);
    output.type = type;
    
    return output;;
}