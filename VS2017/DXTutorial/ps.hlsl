Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambient;
    float4 diffuse;
    float3 dir;
    float padding;
};

struct PixelIn
{
    float4 pos: SV_POSITION;
    float2 uv: TEXCOORD0;
    float3 normal: NORMAL;
};

float4 PS(PixelIn pin) : SV_TARGET
{
    float4 color = shaderTexture.Sample(SampleType, pin.uv);
    color = color * saturate(ambient + diffuse * saturate(dot(pin.normal, -dir)));
    return color;
}