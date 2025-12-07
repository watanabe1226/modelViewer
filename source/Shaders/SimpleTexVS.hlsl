struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
};

cbuffer CameraPos : register(b1)
{
    float3 CameraPos;
    float Pad;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 ray : VECTOR;
    float4 tPos : TPOS;
};

cbuffer Transform : register(b0)
{
    float4x4 World : packoffset(c0);
    float4x4 View : packoffset(c4);
    float4x4 Proj : packoffset(c8);
}

cbuffer LightTransform : register(b3)
{
    float4x4 LightVP;
    float3 LightDir;
    float Padding;
}

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float4 localPos = float4(input.Position, 1.0f);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);
    
    output.Position = projPos;
    output.TexCoord = input.TexCoord;
    output.Normal = normalize(mul((float3x3) World, input.Normal));
    output.tPos = mul(LightVP, worldPos);
    output.ray = normalize(worldPos.xyz - CameraPos);
    
    return output;
}