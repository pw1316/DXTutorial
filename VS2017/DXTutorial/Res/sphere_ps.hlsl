Texture2D shaderTexture : register(t0);
SamplerState SampleType;

cbuffer consts0 : register(b0) {
  float4 Ka;
  float4 Kd;
  float4 Ks;
  float Ns;
};

cbuffer consts1 : register(b1) {
  float4 CameraPos;
  float3 LightDir;
};

struct PixelIn {
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD0;
  float3 normal : NORMAL;
  float3 pos_world : TEXCOORD1;
};

float4 PS(PixelIn pin) : SV_TARGET {
  float4 color = shaderTexture.Sample(SampleType, pin.uv);
  float3 view = normalize(CameraPos.xyz - pin.pos_world);
  float3 ld = normalize(LightDir);
  float3 normal = normalize(pin.normal);
  color = color * saturate(Ka + Kd * saturate(dot(normal, -ld))) +
          Ks * pow(saturate(dot(normalize(view - ld), normal)), Ns);
  return color;
}
