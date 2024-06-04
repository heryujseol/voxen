struct vsOutput
{
    float4 posProj : SV_Position;
    float3 posWorld : POSITION;
    uint face : FACE;
};

cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy1;
    float3 eyeDir;
    float dummy2;
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

cbuffer CloudConstantBuffer : register(b2)
{
    matrix world;
    float3 volumeColor;
    float cloudScale;
}

static const float PI = 3.14159265;
static const float invPI = 1.0 / 3.14159265;

float3 getFaceColor(uint face)
{
    if (face == 0 || face == 1)
    {
        return float3(0.95, 0.95, 0.95);
    }
    else if (face == 4 || face == 5)
    {
        return float3(0.9, 0.9, 0.9);
    }
    else if (face == 3)
    {
        return float3(1.0, 1.0, 1.0);
    }
    else
    {
        return float3(0.75, 0.75, 0.75);
    }
}

float HenyeyGreensteinPhase(float3 L, float3 V, float aniso)
{
    // L: toLight
    // V: eyeDir
    float cosT = dot(L, V);
    float g = aniso;
    return (1.0f - g * g) / (4.0f * PI * pow(abs(1.0f + g * g - 2.0f * g * cosT), 3.0f / 2.0f));
}

float BeerLambert(float absorptionCoefficient, float distanceTraveled)
{
    return exp(-absorptionCoefficient * distanceTraveled);
}

float4 main(vsOutput input) : SV_TARGET
{
    float distance = length(input.posWorld.xz - eyePos.xz);
    
    float sunDirWeight = HenyeyGreensteinPhase(sunDir, eyeDir, 0.625);
    float3 horizonColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    
    // 거리가 멀면 horizon color 선택 
    float horizonWeight = smoothstep(320.0, cloudScale, clamp(distance, 320.0, cloudScale));
    float3 color = volumeColor * getFaceColor(input.face);
    color = lerp(color, horizonColor, horizonWeight);
    
    float sunAltitude = sin(sunDir.y);
    float dayAltitude = PI / 12.0;
    float nightAltitude = -PI * 0.5 * (1.7 / 6.0);
    float maxHorizonAltitude = -PI / 24.0;
    if (dayAltitude < sunAltitude)
    {
        color *= float3(1.0, 1.0, 1.0);
    }
    else if (maxHorizonAltitude < sunAltitude && sunAltitude <= dayAltitude)
    {
        color *= lerp(horizonColor, float3(1, 1, 1), smoothstep(maxHorizonAltitude, dayAltitude, sunAltitude));
    }
    else if (nightAltitude < sunAltitude && sunAltitude <= maxHorizonAltitude)
    {
        color *= lerp(float3(0.04, 0.05, 0.09), horizonColor, smoothstep(nightAltitude, maxHorizonAltitude, sunAltitude));
    }
    else // nightAltitude
    {
        color *= float3(0.04, 0.05, 0.09);
    }
    
    // distance alpha
    float alphaWeight = smoothstep(320.0, cloudScale, clamp(distance, 320.0, cloudScale));
    float alpha = (1.0 - alphaWeight) * 0.8; // [0, 0.8]
    
    return float4(color, alpha);
}