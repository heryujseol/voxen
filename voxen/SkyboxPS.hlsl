Texture2D sunTexture : register(t0);
Texture2D moonTexture : register(t1);

SamplerState pointSampler : register(s0);

cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyePos;
    float dummy;
    float3 eyeDir;
    float dummy2;
}

cbuffer SkyboxConstantBuffer : register(b1)
{
    float3 sunDir;
    float skyScale;
};

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
};

static const float PI = 3.14159265;
static const float invPI = 1.0 / 3.14159265;
static const float showAltitudeBoundary = -0.3;
static const float sectionAltitudeBounary = 0.05;

static float3 horizonDay = float3(0.6, 0.8, 1.0);
static float3 horizonNight = float3(0.05, 0.05, 0.2);
static float3 horizonSunrise = float3(0.8, 0.4, 0.2);
static float3 horizonSunset = float3(0.9, 0.6, 0.2);

static float3 zenithDay = float3(0.3, 0.6, 1.0);
static float3 zenithNight = float3(0.0, 0.0, 0.1);

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

float3 getSkyColor(float posAltitude, float sunAltitude)
{
    // 태양 위치에 따른 빠른 색 변환 (고도가 0에서 증가할 때 빠르게 밤낮이 바뀌기 위함)
    float exp = ((sunAltitude >= 0 ? 1.0 : -1.0) * pow(abs(sunAltitude), 0.6) + 1.0) * 0.5;
    float3 zenithColor = lerp(zenithNight, zenithDay, exp);
    float3 normalHorizonColor = lerp(horizonNight, horizonDay, exp);
    
    // 태양의 고도가 낮을 때만 sun컬러를 결정하도록 선택
    // zenith와 horizon 구별 고도 고려
    float3 sunHorizonX = lerp(horizonSunset, horizonSunrise, (sunDir.x + 1.0) * 0.5);
    float3 sunHorizon = lerp(sunHorizonX, normalHorizonColor, pow(abs(sunAltitude - sectionAltitudeBounary), 0.3));
    
    // 바라보는 방향에 대한 비등방성
    float sunDirWeight = pow(max(dot(sunDir, eyeDir), 0.0), 3.0);
    float3 horizonColor = lerp(normalHorizonColor, sunHorizon, sunDirWeight);
    
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
    
    // ([0, pi] - pi/2) * -2/pi -> [1, -1]
    float posAltitude = clamp((acos(posDir.y) - (PI * 0.5)) * (-2.0 * invPI), -1.0, 1.0);
    float sunAltitude = clamp((acos(sunDir.y) - (PI * 0.5)) * (-2.0 * invPI), -1.0, 1.0);
    
    // sun
    float maxSunSize = 200.0f;
    float minSunSize = 75.0f;
    float sunSize = lerp(minSunSize, maxSunSize, pow(max(dot(sunDir, eyeDir), 0.0), 3.0));
    float2 sunTexcoord;
    if (sunAltitude > showAltitudeBoundary && getPlanetTexcoord(posDir, sunDir, sunSize, sunTexcoord))
    {
        float sunStrength = max(sunAltitude, 0.6);
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
        
        float moonStrength = max(-sunAltitude, 0.1);
        color += moonTexture.SampleLevel(pointSampler, moonTexcoord, 0.0).rgb * moonStrength;
    }
   
    color += getSkyColor(posAltitude, sunAltitude);
    
    return float4(color, 1.0);
}


