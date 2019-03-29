/* MIT License

Copyright (c) 2018 Joker Yough

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/

Texture2D shaderTexture[3] : register(t0);
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
  float3 tangent : TANGENT;
  float3 binormal : BINORMAL;
  float3 pos_world : TEXCOORD1;
};

float4 main(PixelIn pin) : SV_TARGET {
  float4 color1 = shaderTexture[0].Sample(SampleType, pin.uv);
  float4 color2 = shaderTexture[2].Sample(SampleType, pin.uv);
  float4 color = 2 * pow(abs(color1), 2.2) * pow(abs(color2), 2.2);
  float3 view = normalize(CameraPos.xyz - pin.pos_world);
  float3 ld = normalize(LightDir);
  float3 normal = normalize(pin.normal);
  float4 bump = shaderTexture[1].Sample(SampleType, pin.uv) * 2.0 - 1.0;
  normal =
      normalize(bump.x * pin.tangent + bump.y * pin.binormal + bump.z * normal);
  color = color * (Ka + Kd * saturate(dot(normal, -ld))) +
          Ks * pow(saturate(dot(normalize(view - ld), normal)), Ns);
  color = pow(abs(color), 1.0 / 2.2);
  return color;
}
