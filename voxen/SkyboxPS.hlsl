Texture2D sunTexture : register(t0);
Texture2D moonTexture : register(t1);

SamplerState pointSampler : register(s0);

cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy;
}

cbuffer SkyboxConstantBuffer : register(b1)
{
    float3 sunDir;
    float dummy2;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

bool getPlanetTexcoord(float3 posDir, float3 planetDir, out float2 texcoord)
{
    if (length(posDir.xy) == 0.0)
        return false;
    
    float3 posDirHorizontal = normalize(float3(planetDir.xy, posDir.z));
    float3 posDirVertical = normalize(float3(posDir.xy, 0.0));
    
    float dotSH = dot(posDirHorizontal, planetDir);
    float dotSV = dot(posDirVertical, planetDir);
    
    if (dotSH >= 0.98 && dotSV >= 0.98)
    {
        // horizontal tex_x 
        float tex_x = 25.0 * (0.02 + (posDirHorizontal.z < 0 ? 1.0 - dotSH : dotSH - 1.0)); // 25.0 * (0.02 + [-0.02, 0.02])
        
        // vertical tex_y
        float3 crossSV = cross(planetDir, posDirVertical);
        float tex_y = 25.0 * (0.02 + (crossSV.z < 0 ? 1.0 - dotSV : dotSV - 1.0)); // 25.0 * (0.02 + [-0.02, 0.02])
        
        texcoord = float2(tex_x, tex_y);
        return true;
    }
    
    return false;
}

float3 getSkyColor(float3 posDir)
{
    float PI = 3.14159265;
    float invPI = 1.0 / 3.14159265;
    // ([0, pi] - pi/2) * -2/pi -> [-1, 1]
    float sunAltitude = clamp((acos(sunDir.y) - (PI * 0.5)) * (-2.0 * invPI), -1.0, 1.0);
    float posAltitude = clamp((acos(posDir.y) - (PI * 0.5)) * (-2.0 * invPI), -1.0, 1.0);
    
    float3 horizonDay = float3(0.5, 0.7, 1.0);
    float3 zenithDay = float3(0.2, 0.5, 1.0);
    
    float3 horizonNight = float3(0.05, 0.05, 0.2);
    float3 zenithNight = float3(0.0, 0.0, 0.2);
    
    float3 horizonSunrise = float3(0.8, 0.4, 0.2);
    float3 zenithSunrise = float3(0.5, 0.3, 0.5);
    
    float3 horizonSunset = float3(0.9, 0.5, 0.2);
    float3 zenithSunset = float3(0.4, 0.4, 0.6);
    
    float3 retHorizonColor = float3(0.0, 0.0, 0.0);
    float3 retZenithColor = float3(0.0, 0.0, 0.0);
    if (sunAltitude >= 0) // Day
    {
        if (sunDir.x > 0) // Sunrise
        {
            retHorizonColor = lerp(horizonSunrise, horizonDay, sunAltitude);
            retZenithColor = lerp(zenithSunrise, zenithDay, sunAltitude);
        }
        else // Sunset
        {
            retHorizonColor = lerp(horizonSunset, horizonDay, sunAltitude);
            retZenithColor = lerp(zenithSunset, zenithDay, sunAltitude);
        }
    }   
    else // Night
    {
        if (sunDir.x < 0) // Sunset
        {
            retHorizonColor = lerp(horizonSunset, horizonNight, -sunAltitude);
            retZenithColor = lerp(zenithSunset, zenithNight, -sunAltitude);
        }
        else // Sunrise
        {
            retHorizonColor = lerp(horizonSunrise, horizonNight, -sunAltitude);
            retZenithColor = lerp(zenithSunrise, zenithNight, -sunAltitude);
        }
    }
    
    return lerp(retHorizonColor, retZenithColor, posAltitude);
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 posDir = normalize(input.posWorld);
    float3 color = float3(0.0, 0.0, 0.0);
    
    // sun
    float2 sunTexcoord;
    if (getPlanetTexcoord(posDir, sunDir, sunTexcoord))
    {
        color += sunTexture.SampleLevel(pointSampler, sunTexcoord, 0.0).rgb;
    }
    
    // moon
    float2 moonTexcoord;
    if (getPlanetTexcoord(posDir, -sunDir, moonTexcoord))
    {
        uint col = 4;
        uint row = 2;
        
        uint day = 0;
        uint index = day % 8; // 0 ~ 7

        uint2 indexUV = uint2(index % col, index / col); // [0,0]~[3,1]
        
        moonTexcoord += indexUV; // moonTexcoord : [0,0]~[4,2] 
        moonTexcoord = float2(moonTexcoord.x / col, moonTexcoord.y / row); // [4,2]->[1,1]
        color += moonTexture.SampleLevel(pointSampler, moonTexcoord, 0.0).rgb;
    }
    
    color += getSkyColor(posDir);
    
    return float4(color, 1.0);
}


