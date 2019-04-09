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
SamplerState reflect_texture_sampler;

struct PixelIn {
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 pos_reflect : TEXCOORD1;
};

float4 main(PixelIn pin) : SV_TARGET {
  float2 uv;
  uv.x = pin.pos_reflect.x / pin.pos_reflect.w / 2.0f + 0.5f;
  uv.y = -pin.pos_reflect.y / pin.pos_reflect.w / 2.0f + 0.5f;
  float4 color = reflect_texture.Sample(reflect_texture_sampler, uv);
  color = 0.5f * color + 0.5f * float4(1.0f, 1.0f, 1.0f, 1.0f);
  return color;
}