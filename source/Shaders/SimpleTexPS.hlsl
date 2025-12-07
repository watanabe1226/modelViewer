struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 ray : VECTOR;
    float4 tPos : TPOS;
};

struct PSOutput
{
    float4 Color : SV_TARGET;

};

cbuffer Material : register(b2)
{
    float3 Difuuse;
    float Alpha;
    float3 Specular;
    float Shininess;
}

cbuffer LightTransform : register(b3)
{
    float4x4 LightVP;
    float3 LightDir;
    float Padding;
}

SamplerState ColorSmp : register(s0);
SamplerComparisonState ShadowSmp : register(s1);
Texture2D ColorMap : register(t0);
Texture2D ShadowMap : register(t1);

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
    
    float4 texColor = ColorMap.Sample(ColorSmp, input.TexCoord);
    float3 diffuse = texColor;
    
    float3 posFromLightVP = input.tPos.xyz / input.tPos.w;
    float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5f, -0.5f);
    float depthFromLight = ShadowMap.SampleCmp(ShadowSmp, shadowUV, posFromLightVP.z - 0.0005f);
    float shadowWeight = lerp(0.5f, 1.0f, depthFromLight);
    
    output.Color = float4(diffuse * shadowWeight, 1.0f);
    
    return output;
}