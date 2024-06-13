struct vsOutput
{
    float4 position : SV_Position;
    uint face : FACE;
    uint type : TYPE;
};
    
vsOutput main(uint data : DATA)
{
    vsOutput output;
    
    int x = (data >> 23) & 63;
    int y = (data >> 17) & 63;
    int z = (data >> 11) & 63;
    uint face = (data >> 8) & 7;
    uint type = data & 255;
    
    output.position = float4(float(x), float(y), float(z), 1.0);
    output.face = face;
    output.type = type;
    
    return output;
}