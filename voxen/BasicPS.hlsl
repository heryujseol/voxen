SamplerState pointClampSS : register(s0);
Texture2DArray atlasTextureArray : register(t0);

cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy;
}

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    float2 uv : TEXCOORD;
    uint face : FACE;
    uint type : TYPE;
};

float2 getVoxelTexcoord(float3 pos, uint face)
{
    float2 texcoord = float2(0.0, 0.0);
    if (face == 0) // left
    {
        texcoord = float2(abs(pos.z - ceil(pos.z)), abs(pos.y - ceil(pos.y)));
    }
    else if (face == 1) // right
    {
        texcoord = float2(abs(pos.z - floor(pos.z)), abs(pos.y - ceil(pos.y)));
    }
    else if (face == 2) // bottom
    {
        texcoord = float2(abs(pos.x - floor(pos.x)), abs(pos.z - floor(pos.z)));
    }
    else if (face == 3) // top
    {
        texcoord = float2(abs(pos.x - floor(pos.x)), abs(pos.z - ceil(pos.z)));
    }
    else if (face == 4) // front
    {
        texcoord = float2(abs(pos.x - floor(pos.x)), abs(pos.y - ceil(pos.y)));
    }
    else // back
    {
        texcoord = float2(abs(pos.x - ceil(pos.x)), abs(pos.y - ceil(pos.y)));
    }

    return texcoord;
}

float4 main(vsOutput input) : SV_TARGET
{
    float2 texcoord = input.uv; //getVoxelTexcoord(input.posModel, input.face);
    uint index = (input.type - 1) * 6 + input.face;

    //float2 dx = ddx(texcoord);
    //float2 dy = ddy(texcoord);
    //float3 color = atlasTextureArray.SampleGrad(pointClampSS, float3(texcoord, index), -0.0, -0.0).rgb;
    
    float3 color = atlasTextureArray.Sample(pointClampSS, float3(texcoord, index)).xyz;

    return float4(color, 1.0); // Set alpha to 1.0 to ensure full opacity
}