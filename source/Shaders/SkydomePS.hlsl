struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

SamplerState ColorSmp : register(s0);
Texture2D ColorMap : register(t0);

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
   
    float2 uv = float2(-input.TexCoord.x - 0.48f, 1.0f -input.TexCoord.y);
    float3 sky = ColorMap.Sample(ColorSmp, uv).rgb;
    
    float g = 1.0f / 2.0f;
    float3 color = pow(abs(sky), float3(g, g, g));
    output.Color = float4(color, 1.0f);
    
    return output;
}