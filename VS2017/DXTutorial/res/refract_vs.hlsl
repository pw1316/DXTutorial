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
  float4 refract_plane;
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

float f(float x, float y, float a, float b, float ni) {
  float res = (1 - ni * ni) * x * x * (x - a) * (x - a);
  res += x * x * (y - b) * (y - b);
  res -= y * y * ni * ni * (x - a) * (x - a);
  return res;
}

float df(float x, float y, float a, float b, float ni) {
  float res = (1 - ni * ni) * 2 * x * (x - a) * (2 * x - a);
  res += 2 * (y - b) * (y - b);
  res -= 2 * y * y * ni * ni * (x - a);
  return res;
}

VertexOut main(VertexIn vin) {
  VertexOut vout;
  float4 pos = mul(vin.pos, matrix_world);
  float4 view = camera_pos - pos;

  // Clip vertices above the refract plane
  vout.clip_value = 1;  //-dot(pos, refract_plane);

  float b = dot(view.xyz, refract_plane.xyz);
  float a = sqrt(dot(view.xyz, view.xyz) - b * b);
  float y = dot(camera_pos, refract_plane);
  float xl = 0;
  float xr = a;
  for (int i = 0; i < 50; ++i) {
    float mid = (xl + xr) / 2;
    if (f(mid, y, a, b, 1.33f) < 0) {
      xl = mid;
    } else {
      xr = mid;
    }
  }
  float3 ref = view.xyz + refract_plane.xyz * (y / xl * a - b);
  pos.xyz = camera_pos.xyz - ref;

  vout.pos_world = pos;
  pos = mul(pos, matrix_view);
  vout.pos = mul(pos, matrix_proj);
  vout.uv = vin.uv;
  vout.normal = normalize(mul(vin.normal, matrix_world));
  vout.tangent = normalize(mul(vin.tangent, matrix_world));
  vout.binormal = normalize(mul(vin.binormal, matrix_world));
  return vout;
}
