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

#include "model_default.h"

#include <filesystem>
#include <locale>
#include <vector>

#include <DirectX/DDSTextureLoader.h>

#include <entity/frustum.h>
#include <entity/vertex_type.h>
#include <utils/range.h>

namespace naiive::entity {
ModelDefault::ModelDefault(ID3D11Device* device, const std::string raw_path)
    : raw_path_(raw_path),
      translate_(0.0f, 0.0f, 0.0f),
      rotation_pyr_(0.0f, 0.0f, 0.0f) {
  std::filesystem::path path(".");
  path /= raw_path;
  path += ".obj";
  path.make_preferred();

  std::string obj_warning;
  std::string obj_error;
  tinyobj::LoadObj(&attribute_, &shapes_, &materials_, &obj_warning, &obj_error,
                   path.string().c_str(), path.parent_path().string().c_str());
  if (!obj_warning.empty()) {
    LOG(LOG_WARN)
    (raw_path_, "Load model with warning", obj_warning);
  }
  ASSERT_MESSAGE(obj_error.empty())
  (raw_path_, "Load model with error", obj_error);

  Initialize(device);
}

void ModelDefault::Initialize(ID3D11Device* device) {
  HRESULT hr = S_OK;

  num_indices_ = static_cast<UINT>(shapes_[0].mesh.indices.size());
  std::vector<VertexType> vertices(num_indices_);
  std::vector<ULONG> indices(num_indices_);
  for (UINT tri_id = 0; tri_id < num_indices_ / 3; ++tri_id) {
    for (UINT v_id = 0; v_id < 3; ++v_id) {
      vertices[3 * tri_id + v_id].pos = Vertex(tri_id, v_id);
      vertices[3 * tri_id + v_id].uv = TexCoord(tri_id, v_id);
      vertices[3 * tri_id + v_id].normal = Normal(tri_id, v_id);
      indices[3 * tri_id + v_id] = 3 * tri_id + v_id;
      aabb_.Add({vertices[3 * tri_id + v_id].pos.x,
                 +vertices[3 * tri_id + v_id].pos.y,
                 +vertices[3 * tri_id + v_id].pos.z});
    }
    // Calculate T and B
    FLOAT du1 = vertices[3 * tri_id + 1].uv.x - vertices[3 * tri_id].uv.x;
    FLOAT dv1 = vertices[3 * tri_id + 1].uv.y - vertices[3 * tri_id].uv.y;
    FLOAT du2 = vertices[3 * tri_id + 2].uv.x - vertices[3 * tri_id].uv.x;
    FLOAT dv2 = vertices[3 * tri_id + 2].uv.y - vertices[3 * tri_id].uv.y;
    FLOAT dx1 = vertices[3 * tri_id + 1].pos.x - vertices[3 * tri_id].pos.x;
    FLOAT dy1 = vertices[3 * tri_id + 1].pos.y - vertices[3 * tri_id].pos.y;
    FLOAT dz1 = vertices[3 * tri_id + 1].pos.z - vertices[3 * tri_id].pos.z;
    FLOAT dx2 = vertices[3 * tri_id + 2].pos.x - vertices[3 * tri_id].pos.x;
    FLOAT dy2 = vertices[3 * tri_id + 2].pos.y - vertices[3 * tri_id].pos.y;
    FLOAT dz2 = vertices[3 * tri_id + 2].pos.z - vertices[3 * tri_id].pos.z;
    FLOAT coef = 1.0f / (du1 * dv2 - du2 * dv1);
    DirectX::XMFLOAT3X3 lhs = {dv2, -dv1, 0, -du2, du1, 0, 0, 0, 0};
    DirectX::XMFLOAT3X3 rhs = {dx1, dy1, dz1, dx2, dy2, dz2, 0, 0, 0};
    auto xmresult =
        coef * DirectX::XMLoadFloat3x3(&lhs) * DirectX::XMLoadFloat3x3(&rhs);
    DirectX::XMStoreFloat4(&vertices[3 * tri_id].tangent,
                           DirectX::XMVector3Normalize(xmresult.r[0]));
    DirectX::XMStoreFloat4(&vertices[3 * tri_id].binormal,
                           DirectX::XMVector3Normalize(xmresult.r[1]));

    vertices[3 * tri_id + 1].tangent = vertices[3 * tri_id].tangent;
    vertices[3 * tri_id + 1].binormal = vertices[3 * tri_id].binormal;
    vertices[3 * tri_id + 2].tangent = vertices[3 * tri_id].tangent;
    vertices[3 * tri_id + 2].binormal = vertices[3 * tri_id].binormal;
  }

  D3D11_BUFFER_DESC buffer_desc;
  D3D11_SUBRESOURCE_DATA sub_data;
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(VertexType) * num_indices_;
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
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get VertexBuffer failed.");

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(ULONG) * num_indices_;
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
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get IndexBuffer failed.");

  // Texture
  auto texture_name = raw_path_ + "_diffuse.dds";
  WCHAR texture_name_l[128] = {0};
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)(texture_name.size() + 1), texture_name_l, 128);
  hr = DirectX::CreateDDSTextureFromFileEx(
      device, nullptr, texture_name_l, 0, D3D11_USAGE_DEFAULT,
      D3D11_BIND_SHADER_RESOURCE, 0, 0, TRUE, nullptr, &srv_diffuse_map_);
  CHECK(SUCCEEDED(hr))(raw_path_, "Texture [diffuse] missing");

  texture_name = raw_path_ + "_bump.dds";
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)(texture_name.size() + 1), texture_name_l, 128);
  hr = DirectX::CreateDDSTextureFromFile(device, texture_name_l, nullptr,
                                         &srv_bump_map_);
  CHECK(SUCCEEDED(hr))(raw_path_, "Texture [bump] missing");
}
void ModelDefault::Shutdown() {
  SafeRelease(&srv_bump_map_);
  SafeRelease(&srv_diffuse_map_);
  SafeRelease(&index_buffer_);
  SafeRelease(&vertex_buffer_);
}
}  // namespace naiive::entity
