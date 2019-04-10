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

#define INV_E 0.36787944117144232159552377016146f

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
  float4 light_dir;
  float fog_start;
  float fog_end;
  float fog_intensity;
  float padding;
};

struct PixelIn {
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 normal : NORMAL;
  float4 tangent : TANGENT;
  float4 binormal : BINORMAL;
  float4 pos_world : TEXCOORD1;
  float clip_value : SV_ClipDistance0;
};

float4 main(PixelIn pin) : SV_TARGET {
  float4 color1 = shaderTexture[0].Sample(SampleType, pin.uv);
  float4 color2 = shaderTexture[2].Sample(SampleType, pin.uv);
  float4 color;
  if (color2.x == 0 && color2.y == 0 && color2.z == 0 && color2.w == 0) {
    color = color1;
  } else {
    color = 2 * color1 * color2;
  }
  float dist = length(CameraPos - pin.pos_world);
  float4 view = normalize(CameraPos - pin.pos_world);
  float4 normal = normalize(pin.normal);
  float4 tangent = normalize(pin.tangent);
  float4 binormal = normalize(pin.binormal);

  float4 bump = shaderTexture[1].Sample(SampleType, pin.uv) * 2.0 - 1.0;
  if (bump.x != -1 || bump.y != -1 || bump.z != -1)
    normal = normalize(bump.x * tangent + bump.y * binormal + bump.z * normal);
  color = color * (Ka + Kd * saturate(dot(normal, -light_dir))) +
          Ks * pow(saturate(dot(normalize(view - light_dir), normal)), Ns);
  float fog = pow(INV_E, max(0, dist - fog_start) * fog_intensity * 0.1);
  // float fog = saturate((fog_end - dist) / (fog_end - fog_start));
  color = color * fog + (1 - fog) * fog_intensity;
  return color;
}
