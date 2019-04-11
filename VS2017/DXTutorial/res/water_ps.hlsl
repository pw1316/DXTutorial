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

Texture2D reflect_texture : register(t0);
Texture2D refract_texture : register(t1);
SamplerState texture_sampler;

struct PixelIn {
  float4 pos : SV_POSITION;
  float4 pos_reflect : TEXCOORD0;
  float4 pos_refract : TEXCOORD1;
};

float4 main(PixelIn pin) : SV_TARGET {
  float2 uv;
  uv.x = pin.pos_reflect.x / pin.pos_reflect.w / 2.0f + 0.5f;
  uv.y = -pin.pos_reflect.y / pin.pos_reflect.w / 2.0f + 0.5f;
  float4 color_reflect = reflect_texture.Sample(texture_sampler, uv);
  uv.x = pin.pos_refract.x / pin.pos_refract.w / 2.0f + 0.5f;
  uv.y = -pin.pos_refract.y / pin.pos_refract.w / 2.0f + 0.5f;
  float4 color_refract = refract_texture.Sample(texture_sampler, uv);
  float4 color = 0.5f * color_reflect + 0.5f * color_refract;
  return color;
}
