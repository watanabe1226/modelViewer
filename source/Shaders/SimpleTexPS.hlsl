struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PSOutput
{
    float4 Color : SV_TARGET;

};

cbuffer Material : register(b1)
{
    float3 Difuuse;
    float Alpha;
    float3 Specular;
    float Shininess;
}

SamplerState ColorSmp : register(s0);
Texture2D ColorMap : register(t0);

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
   
    output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);
    
    return output;
}