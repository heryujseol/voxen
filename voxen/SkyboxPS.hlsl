Texture2D sunTexture : register(t0);
Texture2D moonTexture : register(t1);

SamplerState pointSampler : register(s0);

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

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

static const float PI = 3.14159265;
static const float invPI = 1.0 / 3.14159265;

bool getPlanetTexcoord(float3 posDir, float3 planetDir, float size, out float2 texcoord)
{
    if (length(posDir.xy) == 0.0)
        return false;
    
    float3 posDirHorizontal = normalize(float3(planetDir.xy, posDir.z));
    float3 posDirVertical = normalize(float3(posDir.xy, 0.0));
    
    float dotSH = max(dot(posDirHorizontal, planetDir), 0.0);
    float dotSV = max(dot(posDirVertical, planetDir), 0.0);
    
    float width = max(tan(acos(dotSH)), 0.0) * skyScale;
    float height = max(tan(acos(dotSV)), 0.0) * skyScale;
    
    if (width <= size && height <= size) // 0 ~ size
    {
        // horizontal tex_x 
        float sign = posDirHorizontal.z > 0 ? -1 : 1;
        float tex_x = (sign * width + size) * 0.5 / size;
        
        // vertical tex_y
        float3 crossSV = cross(planetDir, posDirVertical);
        sign = crossSV.z > 0 ? -1 : 1;
        float tex_y = (sign * height + size) * 0.5 / size;
        
        texcoord = float2(tex_x, tex_y);
        return true;
    }
    
    return false;
}

float3 getSkyColor(float3 posDir)
{
    // ([0, pi] - pi/2) * -2/pi -> [1, -1]
    float posAltitude = clamp((acos(posDir.y) - (PI * 0.5)) * (-2.0 * invPI), -1.0, 1.0);
    
    // zenith와 horizon 구별 고도 고려
    // 최대한 구별된 색 선택하도록 결정
    float3 mixColor = (horizonColor + zenithColor) * 0.5;
    if (posAltitude <= sectionAltitudeBounary)
    {   
        return lerp(horizonColor, mixColor, pow((posAltitude + 1.0) / (1.0 + sectionAltitudeBounary), 10.0));
    }
    else
    {
        return lerp(mixColor, zenithColor, pow((posAltitude - sectionAltitudeBounary) / (1.0 - sectionAltitudeBounary), 0.5));
    }
}

float4 main(vsOutput input) : SV_TARGET
{
    float3 color = float3(0.0, 0.0, 0.0);
    float3 posDir = normalize(input.posWorld);
    
    // sun
    float maxSunSize = 200.0f;
    float minSunSize = 75.0f;
    float sunSize = lerp(minSunSize, maxSunSize, pow(max(dot(sunDir, eyeDir), 0.0), 3.0));
    float2 sunTexcoord;
    if (sunAltitude > showAltitudeBoundary && getPlanetTexcoord(posDir, sunDir, sunSize, sunTexcoord))
    {
        color += sunTexture.SampleLevel(pointSampler, sunTexcoord, 0.0).rgb * sunStrength;
    }
    
    // moon
    float moonSize = minSunSize;
    float2 moonTexcoord;
    if (-sunAltitude > showAltitudeBoundary && getPlanetTexcoord(posDir, -sunDir, moonSize, moonTexcoord))
    {
        uint col = 4;
        uint row = 2;
        
        uint day = 0;
        uint index = day % 8; // 0 ~ 7

        uint2 indexUV = uint2(index % col, index / col); // [0,0]~[3,1]
        
        moonTexcoord += indexUV; // moonTexcoord : [0,0]~[4,2] 
        moonTexcoord = float2(moonTexcoord.x / col, moonTexcoord.y / row); // [4,2]->[1,1]
        
        color += moonTexture.SampleLevel(pointSampler, moonTexcoord, 0.0).rgb * moonStrength;
    }
   
    color += getSkyColor(posDir);
    
    return float4(color, 1.0);
}


