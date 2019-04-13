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

cbuffer consts0 : register(b0) {
  matrix matrix_world;
  matrix matrix_view;
  matrix matrix_proj;
  matrix matrix_reflect_view;
};

struct VertexIn {
  float4 pos : POSITION;
  float2 uv : TEXCOORD0;
};

struct VertexOut {
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 pos_reflect : TEXCOORD1;
  float4 pos_refract : TEXCOORD2;
};

VertexOut main(VertexIn vin) {
  float4 pos_world = mul(vin.pos, matrix_world);
  VertexOut vout;
  vout.pos = mul(pos_world, matrix_view);
  vout.pos = mul(vout.pos, matrix_proj);
  vout.uv = vin.uv;
  vout.pos_reflect = mul(pos_world, matrix_reflect_view);
  vout.pos_reflect = mul(vout.pos_reflect, matrix_proj);
  vout.pos_refract = vout.pos;  // Ni=1 for now
  return vout;
}
