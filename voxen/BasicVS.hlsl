cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
}

cbuffer ChunkConstantBuffer : register(b1)
{
    matrix world;
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float2 uv : TEXCOORD;
    uint face : FACE;
    uint type : TYPE;
};

float2 getVoxelTexcoord(float3 pos, uint face)
{
    float2 texcoord = float2(0.0, 0.0);
    if (face == 0) // left
    {
        texcoord = float2(pos.z, pos.y);
    }
    else if (face == 1) // right
    {
        texcoord = float2(pos.z, pos.y);
    }
    else if (face == 2) // bottom
    {
        texcoord = float2(pos.x, pos.z);
    }
    else if (face == 3) // top
    {
        texcoord = float2(pos.x, pos.z);
    }
    else if (face == 4) // front
    {
        texcoord = float2(pos.x, pos.y);
    }
    else // back
    {
        texcoord = float2(pos.x, pos.y);
    }

    return -texcoord;
}

vsOutput main(uint data : DATA)
{
    vsOutput output;
    
    int x = (data >> 23) & 63;
    int y = (data >> 17) & 63;
    int z = (data >> 11) & 63;
    uint face = (data >> 8) & 7;
    uint type = data & 255;
    
    float3 position = float3(float(x), float(y), float(z));
    output.posWorld = mul(float4(position, 1.0), world).xyz;
  
    output.posProj = float4(output.posWorld, 1.0);
    output.posProj = mul(output.posProj, view); 
    output.posProj = mul(output.posProj, proj);
    
    output.face = face;
    output.type = type;
    
    output.uv = getVoxelTexcoord(position, face);
    
    return output;
}