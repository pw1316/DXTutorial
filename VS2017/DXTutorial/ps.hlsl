Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 Ka;
    float4 Kd;
    float4 Ks;
    float3 lightDir;
    float Ns;
};

struct PixelIn
{
    float4 pos: SV_POSITION;
    float2 uv: TEXCOORD0;
    float3 normal: NORMAL;
    float3 view: TEXCOORD1;
};

float4 PS(PixelIn pin) : SV_TARGET
{
    float4 color = shaderTexture.Sample(SampleType, pin.uv);
    color = color * saturate(Ka + Kd * saturate(dot(pin.normal, -lightDir)) + Ks * saturate(pow(dot(normalize(pin.view - lightDir), pin.normal), Ns)));
    return color;
}