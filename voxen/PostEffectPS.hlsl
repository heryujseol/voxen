Texture2D renderTex : register(t0); // Rendering results
Texture2D depthOnlyTex : register(t1); // DepthOnly

SamplerState linearClampSS : register(s0);

cbuffer CameraConstantBuffer : register(b0)
{
    Matrix view;
    Matrix proj;
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

float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    ////Beer-Lambert law
    float3 fogColor = float3(1, 1, 1);
    float fogMin = 280.0;
    float fogMax = 550.0;
    float fogStrength = 2.0;
        
    float4 posView = TexcoordToView(input.texcoord);
    float dist = length(posView.xyz);
        
    float distFog = saturate((dist - fogMin) / (fogMax - fogMin));
    float fogFactor = exp(-distFog * fogStrength);
        
    float3 color = renderTex.Sample(linearClampSS, input.texcoord).rgb;
        
    color = lerp(fogColor, color, fogFactor);
        
    return float4(color, 1.0);
    
    //float z = TexcoordToView(input.texcoord).z * 0.1;
    //return float4(z, z, z, 1);
    
    //return float4(1.0, 0.0, 0.0, 1.0);
}