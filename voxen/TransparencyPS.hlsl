Texture2DArray atlasTextureArray : register(t0);
Texture2D mirrorWorldTex : register(t1);
Texture2D depthOnlyTex : register(t2);
Texture2D basicRenderTex : register(t3);

SamplerState pointWrapSS : register(s0);
SamplerState linearClampSS : register(s2);

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

struct vsOutput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION1;
    sample float3 posModel : POSITION2;
    uint face : FACE;
    uint type : TYPE;
};

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

float3 schlickFresnel(float3 N, float3 E, float3 R)
{
    // https://en.wikipedia.org/wiki/Schlick%27s_approximation
    // [f0 ~ 1]
    // 90µµ -> dot(N,E)==0 -> f0+(1-f0)*1^5 -> 1
    //  0µµ -> dot(N,E)==1 -> f0+(1-f0)*0*5 -> f0
    return R + (1 - R) * pow((1 - max(dot(N, E), 0.0)), 5.0);
}

float3 texcoordToView(float2 texcoord)
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
    
    return posView.xyz;
}

float4 main(vsOutput input) : SV_TARGET
{
    float2 texcoord = getVoxelTexcoord(input.posModel, input.face);
    uint index = (input.type - 1) * 6 + input.face;
    
    float3 normal = getNormal(input.face);
    
    if (normal.y > 0 && 62 - 1e4 <= input.posWorld.y && input.posWorld.y <= 62 + 1e4)
    {
        float2 screenTexcoord = float2(input.posProj.x / 1920.0, input.posProj.y / 1080.0);
        
        // origin render color
        float3 originColor = basicRenderTex.Sample(linearClampSS, screenTexcoord).rgb;
        
        // absorption color
        float3 textureColor = atlasTextureArray.Sample(pointWrapSS, float3(texcoord, index)).rgb;
        
        // reflect color
        float4 mirrorColor = mirrorWorldTex.Sample(linearClampSS, screenTexcoord);
        
        // fresnel factor
        float3 toEye = normalize(eyePos - input.posWorld);
        float3 reflectCoeff = float3(0.2, 0.2, 0.2);
        float3 fresnelFactor = schlickFresnel(normal, toEye, reflectCoeff);
        
        // absorption factor
        float objectDistance = length(texcoordToView(screenTexcoord));
        float planeDistance = length(eyePos - input.posWorld);
        float diffDistance = abs(objectDistance - planeDistance);
        float absorptionCoeff = 0.2;
        float absorptionFactor = 1.0 - exp(-absorptionCoeff * diffDistance); // beer-lambert
        
        // blending 3 colors
        float3 projColor = (1.0 - fresnelFactor) * (lerp(originColor, textureColor, absorptionFactor));
        float3 blendColor = lerp(projColor, mirrorColor.rgb, fresnelFactor);
        
        // alpha blend
        float alpha = lerp(1.0, mirrorColor.a, fresnelFactor);
        
        return float4(blendColor, alpha);
    }
    else
    {
        discard;
        return float4(0, 0, 0, 0);
    }
}