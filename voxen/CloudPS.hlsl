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
    float3 sunStrength;
    float sunAltitude;
    float3 moonStrength;
    float sectionAltitudeBounary;
    float3 horizonColor;
    float showAltitudeBoundary;
    float3 zenithColor;
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
    
   /*
    // lightColor
    float cloudAltitude = clamp((saturate(192.0 / skyScale) - (PI * 0.5)) * (-2.0 * invPI), -1.0, 1.0);
    float3 mixColor = (horizonColor + zenithColor) * 0.5;
    float3 lightColor = float3(0.0, 0.0, 0.0);
    if (cloudAltitude <= sectionAltitudeBounary)
    {
        lightColor = lerp(horizonColor, mixColor, pow((cloudAltitude + 1.0) / (1.0 + sectionAltitudeBounary), 10.0));
    }
    else
    {
        lightColor = lerp(mixColor, zenithColor, pow((cloudAltitude - sectionAltitudeBounary) / (1.0 - sectionAltitudeBounary), 0.5));
    }
    lightColor = horizonColor;
    */
    // lighting
    //float3 lighting = sunStrength * 15.0 * getFaceColor(input.face);
    
    //color = volumeColor * density * lightColor;
    //float3 eyeToPos = 
    //color = lightColor * lighting * density * HenyeyGreensteinPhase(sunDir, eyeToPos, 0.1);
    
    //float distance = length(input.posWorld.xz - eyePos.xz);
    //float alpha = lerp(0.0, 1.0, (320.0 - distance) / 320.0);
    
    //color += volumeColor * 4.0 * lerp(horizonColor, zenithColor, 0.5) * density * getFaceColor(input.face) * BeerLambert(density, 1.0);
    float3 color = volumeColor * getFaceColor(input.face);
    
    float distance = length(input.posWorld.xz - eyePos.xz);
    
    float horizonWeight = smoothstep(260.0, cloudScale, clamp(distance, 260.0, cloudScale));
    color = lerp(color, horizonColor, horizonWeight);
    
    if (sunAltitude >= sectionAltitudeBounary) // day
    {

    }
    else if (sunAltitude <= showAltitudeBoundary) // night
    {
      
    }
    else // sunset or sunrise
    {
       
    }
    
    //float alphaWeight = smoothstep(0.0, 1.0, saturate((cloudScale - distance) / cloudScale));
    //float alpha = alphaWeight * 0.85;
    float alphaWeight = smoothstep(260.0, cloudScale, clamp(distance, 260.0, cloudScale));
    float alpha = (1.0 - alphaWeight) * 0.8; // [0, 0.85]
    
    return float4(color, alpha);
}