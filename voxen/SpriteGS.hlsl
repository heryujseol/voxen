cbuffer CameraConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
}

cbuffer ChunkConstantBuffer : register(b1)
{
    matrix world;
}

struct vsOutput
{
    float4 posModel : SV_Position;
    uint type : TYPE;
};

struct gsOutput
{
    float4 posProj : SV_Position;
    sample float2 texcoord : TEXCOORD;
    uint type : TYPE;
};

[maxvertexcount(16)]
void main(point vsOutput input[1], inout TriangleStream<gsOutput> outputStream)
{
    gsOutput output;
    output.type = input[0].type;
    
    float3 centerPos = input[0].posModel.xyz + float3(0.5, 0.5, 0.5);
    
    float width = 1.0;
    float height = 1.0;
    
    float3 v = centerPos + float3(-width * 0.5, -height * 0.5, -width * 0.5);
    output.posProj = mul(float4(v, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    output.texcoord = float2(0.0, 1.0);
    outputStream.Append(output);
    
    v = centerPos + float3(-width * 0.5, height * 0.5, -width * 0.5);
    output.posProj = mul(float4(v, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    output.texcoord = float2(0.0, 0.0);
    outputStream.Append(output);
    
    v = centerPos + float3(width * 0.5, -height * 0.5, width * 0.5);
    output.posProj = mul(float4(v, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    output.texcoord = float2(1.0, 1.0);
    outputStream.Append(output);
    
    v = centerPos + float3(width * 0.5, height * 0.5, width * 0.5);
    output.posProj = mul(float4(v, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    output.texcoord = float2(1.0, 0.0);
    outputStream.Append(output);
    
    outputStream.RestartStrip();
    
    v = centerPos + float3(-width * 0.5, -height * 0.5, width * 0.5);
    output.posProj = mul(float4(v, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    output.texcoord = float2(0.0, 1.0);
    outputStream.Append(output);
    
    v = centerPos + float3(-width * 0.5, height * 0.5, width * 0.5);
    output.posProj = mul(float4(v, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    output.texcoord = float2(0.0, 0.0);
    outputStream.Append(output);
    
    v = centerPos + float3(width * 0.5, -height * 0.5, -width * 0.5);
    output.posProj = mul(float4(v, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    output.texcoord = float2(1.0, 1.0);
    outputStream.Append(output);
    
    v = centerPos + float3(width * 0.5, height * 0.5, -width * 0.5);
    output.posProj = mul(float4(v, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    output.texcoord = float2(1.0, 0.0);
    outputStream.Append(output);
}