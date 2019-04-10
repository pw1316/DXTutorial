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
};

cbuffer consts1 : register(b1) {
  float4 camera_pos;
  float4 light_dir;
  float fog_start;
  float fog_end;
  float fog_intensity;
  float padding;
  float4 clip_plane;
};

struct VertexIn {
  float4 pos : POSITION;
  float2 uv : TEXCOORD0;
  float4 normal : NORMAL;
  float4 tangent : TANGENT;
  float4 binormal : BINORMAL;
};

struct VertexOut {
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 normal : NORMAL;
  float4 tangent : TANGENT;
  float4 binormal : BINORMAL;
  float4 pos_world : TEXCOORD1;
  float clip_value : SV_ClipDistance0;
};

VertexOut main(VertexIn vin) {
  VertexOut vout;
  float4 pos;
  pos = mul(vin.pos, matrix_world);
  vout.pos_world = pos;
  pos = mul(pos, matrix_view);
  vout.pos = mul(pos, matrix_proj);
  vout.uv = vin.uv;
  vout.normal = normalize(mul(vin.normal, matrix_world));
  vout.tangent = normalize(mul(vin.tangent, matrix_world));
  vout.binormal = normalize(mul(vin.binormal, matrix_world));
  vout.clip_value = dot(clip_plane, vout.pos_world);
  return vout;
}
