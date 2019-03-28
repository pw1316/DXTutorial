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
  matrix MatrixWorld;
  matrix MatrixView;
  matrix MatrixProj;
};

cbuffer consts1 : register(b1) {
  float4 CameraPos;
  float3 LightDir;
};

struct VertexIn {
  float4 pos : POSITION;
  float2 uv : TEXCOORD0;
  float3 normal : NORMAL;
};

struct VertexOut {
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD0;
  float3 normal : NORMAL;
  float3 pos_world : TEXCOORD1;
};

VertexOut main(VertexIn vin) {
  VertexOut vout;
  vin.pos.w = 1;
  float4 pos;
  pos = mul(vin.pos, MatrixWorld);
  vout.pos_world = pos.xyz;
  pos = mul(pos, MatrixView);
  vout.pos = mul(pos, MatrixProj);
  vout.uv = vin.uv;
  vout.normal = normalize(mul(vin.normal, (float3x3)MatrixWorld));
  return vout;
}
