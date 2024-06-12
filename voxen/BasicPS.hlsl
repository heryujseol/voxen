Texture2DArray atlasTextureArray : register(t0);
Texture2D grassColorMap : register(t1);

SamplerState pointWrapSS : register(s0);

cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy;
    float3 eyeDir;
}

cbuffer SkyboxConstantBuffer : register(b1)
{
    float3 sunDir;
    float skyScale;
    float3 normalHorizonColor;
    uint dateTime;
    float3 normalZenithColor;
    float sunStrength;
    float3 sunHorizonColor;
    float moonStrength;
    float3 sunZenithColor;
    float dummy3;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
};

static const float PI = 3.14159265;

float2 getVoxelTexcoord(float3 pos, uint face)
{
    float2 texcoord = float2(0.0, 0.0);
    
    if (face == 0) // left
    {
        texcoord = float2(-pos.z + 32.0, -pos.y + 32.0);
    }
    else if (face == 1) // right
    {
        texcoord = float2(pos.z, -pos.y + 32.0);
    }
    else if (face == 2) // bottom
    {
        texcoord = float2(pos.x, pos.z);
    }
    else if (face == 3) // top
    {
        texcoord = float2(pos.x, -pos.z + 32.0);
    }
    else if (face == 4) // front
    {
        texcoord = float2(pos.x, -pos.y + 32.0);
    }
    else // back
    {
        texcoord = float2(-pos.x + 32.0, -pos.y + 32.0);
    }

    return texcoord;
}

float3 getNormal(uint face)
{
    if (face == 0)
    {
        return float3(-1.0, 0.0, 0.0);
    }
    else if (face == 1)
    {
        return float3(1.0, 0.0, 0.0);
    }
    else if (face == 2)
    {
        return float3(0.0, -1.0, 0.0);
    }
    else if (face == 3)
    {
        return float3(0.0, 1.0, 0.0);
    }
    else if (face == 4)
    {
        return float3(0.0, 0.0, -1.0);
    }
    else
    {
        return float3(0.0, 0.0, 1.0);
    }
}

float3 getFaceColor(uint face, float3 color)
{
    //if (face == 0 || face == 1)
    //{
    //    return color * 0.75;
    //}
    //else
    if (face == 2)
    {
        return color * 0.25;
    }
    else if (face == 4 || face == 5)
    {
        return color * 0.5;
    }
    else
    {
        return color;
    }
}

float4 main(vsOutput input) : SV_TARGET
{

    //float temperature = 0.5;
    //float downfall = 1.0;
    //float4 biome = grassColorMap.SampleLevel(pointClampSS, float2(1 - temperature, 1 - temperature / downfall), 0.0);
    
    float2 texcoord = getVoxelTexcoord(input.posModel, input.face);
    uint index = (input.type - 1) * 6 + input.face;
    
    float3 color = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, index)).rgb;
    //color = getFaceColor(input.face, color);
    
    float3 normal = getNormal(input.face);
    float ndotl = max(dot(sunDir, normal), 0.3);
    
    float strength = clamp(sunStrength, 0.25, 1.0);
    
    float sunAltitude = sin(sunDir.y);
    float showSectionAltitude = -PI * 0.5 * (1.7 / 6.0);
    
    float3 EyeDir = float3(eyePos - input.posWorld);
    float3 H = normalize(-EyeDir + -sunDir);
    float hdotn = dot(H, normal);
    float3 spec = pow(max(hdotn, 0.0), 4.0);
    
    //if (input.face == 0 || input.face == 1 || input.face == 3)
    //{
        if (12700 <= dateTime && dateTime <= 13700)
        {
            float w = (dateTime - 12700) / 1000.0;
            color = lerp(color * ((strength * ndotl) + spec), color * (strength * 0.3), w);
        }
        else if (22300 <= dateTime && dateTime <= 23300)
        {
            float w = (dateTime - 22300) / 1000.0;
            color = lerp(color * (strength * 0.3), color * (strength * ndotl + spec), w);
        }
        else if (13700 < dateTime && dateTime < 22300)
        {
            color = color * strength * 0.3;
        }
        else
        {
            color = color * ((strength * ndotl) + spec);
        }
    //}
    //else
    //{
    //    color = color * strength;
    //}

    return float4(color, 0.0);
}