Texture2D shaderTexture;
SamplerState SampleType;

struct PixelIn
{
    float4 pos: SV_POSITION;
    float2 uv: TEXCOORD0;
};

float4 PS(PixelIn pin) : SV_TARGET
{
    float4 color = shaderTexture.Sample(SampleType, pin.uv);
    return color;
}