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

#include "model.h"

#include <locale>
#include <vector>

#include <DirectX/DDSTextureLoader.h>

#include <entity/frustum.h>
#include <entity/vertex_type.h>
#include <utils/range.h>

namespace naiive::entity {
void Model3D::Initialize(ID3D11Device* device) {
  InitializeBuffer(device);
}

void Model3D::Shutdown() {
  ShutdownBuffer();
}

void Model3D::InitializeBuffer(ID3D11Device* device) {
  BOOL bool_res = FALSE;
  HRESULT hr = S_OK;

  bool_res = tinyobj::LoadObj(&obj.attr, &obj.shapes, &obj.materials, nullptr,
                              nullptr, (name_ + ".obj").c_str(), "res/", true);
  ASSERT(bool_res);

  D3D11_BUFFER_DESC buffer_desc;
  D3D11_SUBRESOURCE_DATA sub_data;

  /* =====VB & IB===== */
  vertex_number_ = static_cast<UINT>(obj.shapes[0].mesh.indices.size());
  std::vector<VertexType> vertices(vertex_number_);
  std::vector<ULONG> indices(vertex_number_);
  for (UINT triId = 0; triId < vertex_number_ / 3; ++triId) {
    for (UINT j = 0; j < 3; ++j) {
      // RH order to LH order
      auto vi = obj.shapes[0].mesh.indices[3 * triId + 2 - j].vertex_index;
      auto ti = obj.shapes[0].mesh.indices[3 * triId + 2 - j].texcoord_index;
      auto ni = obj.shapes[0].mesh.indices[3 * triId + 2 - j].normal_index;
      auto x = obj.attr.vertices[3 * vi + 0];
      auto y = obj.attr.vertices[3 * vi + 1];
      auto z = obj.attr.vertices[3 * vi + 2];
      aabb_.Add({x, y, -z});
      vertices[3 * triId + j].pos = {x, y, -z, 1.0f};  // Reverse z in position
      x = obj.attr.texcoords[2 * ti + 0];
      y = obj.attr.texcoords[2 * ti + 1];
      vertices[3 * triId + j].uv = {x, 1.0f - y};  // Invert v in texture
      x = obj.attr.normals[3 * ni + 0];
      y = obj.attr.normals[3 * ni + 1];
      z = obj.attr.normals[3 * ni + 2];
      vertices[3 * triId + j].normal = {x, y, -z, 0.0f};  // Reverse z in normal
      indices[3 * triId + j] = 3 * triId + j;
    }
    /* Calculate T and B */
    FLOAT du1 = vertices[3 * triId + 1].uv.x - vertices[3 * triId].uv.x;
    FLOAT dv1 = vertices[3 * triId + 1].uv.y - vertices[3 * triId].uv.y;
    FLOAT du2 = vertices[3 * triId + 2].uv.x - vertices[3 * triId].uv.x;
    FLOAT dv2 = vertices[3 * triId + 2].uv.y - vertices[3 * triId].uv.y;
    FLOAT dx1 = vertices[3 * triId + 1].pos.x - vertices[3 * triId].pos.x;
    FLOAT dy1 = vertices[3 * triId + 1].pos.y - vertices[3 * triId].pos.y;
    FLOAT dz1 = vertices[3 * triId + 1].pos.z - vertices[3 * triId].pos.z;
    FLOAT dx2 = vertices[3 * triId + 2].pos.x - vertices[3 * triId].pos.x;
    FLOAT dy2 = vertices[3 * triId + 2].pos.y - vertices[3 * triId].pos.y;
    FLOAT dz2 = vertices[3 * triId + 2].pos.z - vertices[3 * triId].pos.z;
    FLOAT coef = 1.0f / (du1 * dv2 - du2 * dv1);
    DirectX::XMFLOAT3X3 lhs = {dv2, -dv1, 0, -du2, du1, 0, 0, 0, 0};
    DirectX::XMFLOAT3X3 rhs = {dx1, dy1, dz1, dx2, dy2, dz2, 0, 0, 0};
    auto xmresult =
        coef * DirectX::XMLoadFloat3x3(&lhs) * DirectX::XMLoadFloat3x3(&rhs);
    DirectX::XMStoreFloat4(&vertices[3 * triId].tangent,
                           DirectX::XMVector3Normalize(xmresult.r[0]));
    DirectX::XMStoreFloat4(&vertices[3 * triId].binormal,
                           DirectX::XMVector3Normalize(xmresult.r[1]));

    vertices[3 * triId + 1].tangent = vertices[3 * triId].tangent;
    vertices[3 * triId + 1].binormal = vertices[3 * triId].binormal;
    vertices[3 * triId + 2].tangent = vertices[3 * triId].tangent;
    vertices[3 * triId + 2].binormal = vertices[3 * triId].binormal;
  }

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(VertexType) * vertex_number_;
  buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  buffer_desc.CPUAccessFlags = 0;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  ZeroMemory(&sub_data, sizeof(sub_data));
  sub_data.pSysMem = vertices.data();
  sub_data.SysMemPitch = 0;
  sub_data.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&buffer_desc, &sub_data, &vertex_buffer_);
  ASSERT(SUCCEEDED(hr));

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(ULONG) * vertex_number_;
  buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  buffer_desc.CPUAccessFlags = 0;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  ZeroMemory(&sub_data, sizeof(sub_data));
  sub_data.pSysMem = indices.data();
  sub_data.SysMemPitch = 0;
  sub_data.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&buffer_desc, &sub_data, &index_buffer_);
  ASSERT(SUCCEEDED(hr));

  /* =====Texture=====*/
  auto texture_name = name_ + "_diffuse.dds";
  WCHAR texture_name_l[128] = {0};
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)(texture_name.size() + 1), texture_name_l, 128);
  hr = DirectX::CreateDDSTextureFromFileEx(
      device, nullptr, texture_name_l, 0, D3D11_USAGE_DEFAULT,
      D3D11_BIND_SHADER_RESOURCE, 0, 0, TRUE, nullptr,
      &shader_resource_texture_[0]);
  CHECK(SUCCEEDED(hr))("Texture [diffuse] missing");

  texture_name = name_ + "_bump.dds";
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)(texture_name.size() + 1), texture_name_l, 128);
  hr = DirectX::CreateDDSTextureFromFile(device, texture_name_l, nullptr,
                                         &shader_resource_texture_[1]);
  CHECK(SUCCEEDED(hr))("Texture [bump] missing");

  texture_name = name_ + "_dirt.dds";
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)(texture_name.size() + 1), texture_name_l, 128);
  hr = DirectX::CreateDDSTextureFromFileEx(
      device, nullptr, texture_name_l, 0, D3D11_USAGE_DEFAULT,
      D3D11_BIND_SHADER_RESOURCE, 0, 0, TRUE, nullptr,
      &shader_resource_texture_[2]);
  CHECK(SUCCEEDED(hr))("Texture [dirt] missing");

  /* =====SamplerState===== */
}
void Model3D::ShutdownBuffer() {
  SafeRelease(&shader_resource_texture_[2]);
  SafeRelease(&shader_resource_texture_[1]);
  SafeRelease(&shader_resource_texture_[0]);
  SafeRelease(&index_buffer_);
  SafeRelease(&vertex_buffer_);
}
}  // namespace naiive::entity
