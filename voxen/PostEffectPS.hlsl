Texture2D renderTex : register(t0); // Rendering results
Texture2D depthOnlyTex : register(t1); // DepthOnly

SamplerState linearClampSS : register(s0);

cbuffer CameraConstantBuffer : register(b0)
{
    Matrix view;
    Matrix proj;
    float3 eyePos;
    float dummy1;
    float3 eyeDir;
    float dummy2;
    Matrix invProj;
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

struct SamplingPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

static const float PI = 3.14159265;

float4 TexcoordToView(float2 texcoord)
{
    float4 posProj;

    // [0, 1]x[0, 1] -> [-1, 1]x[-1, 1]
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1;
    posProj.z = depthOnlyTex.Sample(linearClampSS, texcoord).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w; // homogeneous coordinates
    
    return posView;
}

float HenyeyGreensteinPhase(float3 L, float3 V, float aniso)
{
	// L: toLight
	// V: eyeDir
	// https://www.shadertoy.com/view/7s3SRH
    float cosT = dot(L, V);
    float g = aniso;
    return (1.0 - g * g) / (4.0 * PI * pow(abs(1.0 + g * g - 2.0 * g * cosT), 3.0 / 2.0));
}


float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    //Beer-Lambert law
    float3 fogColor = normalHorizonColor;
    float fogMin = 280.0;
    float fogMax = 320.0;
    float fogStrength = 3.0;
        
    float4 posView = TexcoordToView(input.texcoord);
    float dist = length(posView.xyz);
        
    float distFog = saturate((dist - fogMin) / (fogMax - fogMin));
    float fogFactor = exp(-distFog * fogStrength);
        
    float3 color = renderTex.Sample(linearClampSS, input.texcoord).rgb;
    
    if (11000 <= dateTime && dateTime <= 14000)
    {
        float sunDirWeight = HenyeyGreensteinPhase(sunDir, eyeDir, 0.625);
        fogColor = lerp(normalHorizonColor, sunHorizonColor, sunDirWeight);
    }
        
    color = lerp(fogColor, color, fogFactor);
    
    return float4(color, 1.0);
}