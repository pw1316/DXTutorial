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
#include <d3dcompiler.h>

#include <entity/frustum.h>
#include <utils/range.h>

#ifndef NAIIVE_FRUSTUM_CULL
#define NAIIVE_FRUSTUM_CULL 1
#endif

namespace naiive::entity {
void Model3D::Initialize(ID3D11Device* device) {
  InitializeBuffer(device);
  InitializeShader(device);
}

void Model3D::Shutdown() {
  ShutdownShader();
  ShutdownBuffer();
}

BOOL Model3D::Render(ID3D11DeviceContext* context,
                     const DirectX::XMFLOAT4X4& view,
                     const DirectX::XMFLOAT4X4& proj,
                     const DirectX::XMFLOAT4& camera_pos,
                     const DirectX::XMFLOAT3& dir) {
  HRESULT hr = S_OK;
  D3D11_MAPPED_SUBRESOURCE mapped{};

  auto xmworld = DirectX::XMMatrixRotationRollPitchYaw(
      rotation_pyr_.x, rotation_pyr_.y, rotation_pyr_.z);
  xmworld = DirectX::XMMatrixMultiply(
      xmworld,
      DirectX::XMMatrixTranslation(translate_.x, translate_.y, translate_.z));
  auto xmview = DirectX::XMLoadFloat4x4(&view);
  auto xmproj = DirectX::XMLoadFloat4x4(&proj);
  DirectX::XMFLOAT4X4 matrix_view_proj;
  DirectX::XMStoreFloat4x4(&matrix_view_proj,
                           DirectX::XMMatrixMultiply(xmview, xmproj));
#if NAIIVE_FRUSTUM_CULL
  FrustumWorld frustum(matrix_view_proj);
  BOOL visible = FALSE;
  for (auto i : utils::Range(8)) {
    auto cnr = aabb_.Corner(i);
    auto xmcnr =
        DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&cnr), xmworld);
    DirectX::XMStoreFloat3(&cnr, xmcnr);
    if (frustum.Check(cnr)) {
      visible = TRUE;
      break;
    }
  }
  if (!visible) {
    return FALSE;
  }
#endif
  context->Map(const_buffer_transform_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (CBTransformType*)mapped.pData;

    DirectX::XMStoreFloat4x4(&rawdata->world,
                             DirectX::XMMatrixTranspose(xmworld));
    DirectX::XMStoreFloat4x4(&rawdata->view,
                             DirectX::XMMatrixTranspose(xmview));
    DirectX::XMStoreFloat4x4(&rawdata->proj,
                             DirectX::XMMatrixTranspose(xmproj));
  }
  context->Unmap(const_buffer_transform_, 0);

  context->Map(const_buffer_camera_light_, 0, D3D11_MAP_WRITE_DISCARD, 0,
               &mapped);
  {
    auto rawdata = (CBCameraLightType*)mapped.pData;
    rawdata->camera_pos = camera_pos;
    rawdata->light_dir = dir;
  }
  context->Unmap(const_buffer_camera_light_, 0);

  UINT stride = sizeof(VBType);
  UINT offset = 0;
  context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
  context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetInputLayout(input_layout_);

  context->VSSetConstantBuffers(0, 1, &const_buffer_transform_);
  context->VSSetConstantBuffers(1, 1, &const_buffer_camera_light_);
  context->VSSetShader(vertex_shader_, nullptr, 0);

  context->PSSetConstantBuffers(0, 1, &const_buffer_material_);
  context->PSSetConstantBuffers(1, 1, &const_buffer_camera_light_);
  context->PSSetShaderResources(0, 1, &shader_resource_texture_);
  context->PSSetSamplers(0, 1, &sampler_state_);
  context->PSSetShader(pixel_shader_, nullptr, 0);

  context->DrawIndexed(vertex_number_, 0, 0);
  return TRUE;
}

void Model3D::InitializeBuffer(ID3D11Device* device) {
  HRESULT hr = S_OK;

  TinyObj obj;
  hr = tinyobj::LoadObj(&obj.attr, &obj.shapes, &obj.materials, nullptr,
                        nullptr, (name_ + ".obj").c_str(), "Res/", true)
           ? S_OK
           : E_FAIL;
  FAILTHROW;

  D3D11_BUFFER_DESC buffer_desc;
  D3D11_SUBRESOURCE_DATA sub_data;
  D3D11_SAMPLER_DESC sampler_desc;

  /* =====VB & IB===== */
  vertex_number_ = static_cast<UINT>(obj.shapes[0].mesh.indices.size());
  std::vector<VBType> vertices(vertex_number_);
  std::vector<ULONG> indices(vertex_number_);
  for (std::remove_const<decltype(vertex_number_)>::type triId = 0;
       triId < vertex_number_ / 3; ++triId) {
    for (UINT j = 0; j < 3; ++j) {
      auto vi = obj.shapes[0].mesh.indices[3 * triId + 2 - j].vertex_index;
      auto ti = obj.shapes[0].mesh.indices[3 * triId + 2 - j].texcoord_index;
      auto ni = obj.shapes[0].mesh.indices[3 * triId + 2 - j].normal_index;
      auto x = obj.attr.vertices[3 * vi + 0];
      auto y = obj.attr.vertices[3 * vi + 1];
      auto z = obj.attr.vertices[3 * vi + 2];
      aabb_.Add({x, y, -z});
      vertices[3 * triId + j].pos = {x, y, -z};
      x = obj.attr.texcoords[2 * ti + 0];
      y = obj.attr.texcoords[2 * ti + 1];
      vertices[3 * triId + j].uv = {x, 1.0f - y};
      x = obj.attr.normals[3 * ni + 0];
      y = obj.attr.normals[3 * ni + 1];
      z = obj.attr.normals[3 * ni + 2];
      vertices[3 * triId + j].normal = {x, y, -z};
      indices[3 * triId + j] = 3 * triId + j;
    }
  }
  // vertex_number_ = 3;
  // std::vector<VBType> vertices(vertex_number_);
  // std::vector<ULONG> indices(vertex_number_);
  // vertices[0].pos = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);
  // vertices[0].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);
  // vertices[0].uv = DirectX::XMFLOAT2(0.0f, 0.0f);
  // indices[0] = 0;
  // vertices[1].pos = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);
  // vertices[1].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);
  // vertices[1].uv = DirectX::XMFLOAT2(1.0f, 0.0f);
  // indices[1] = 1;
  // vertices[2].pos = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);
  // vertices[2].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);
  // vertices[2].uv = DirectX::XMFLOAT2(1.0f, 1.0f);
  // indices[2] = 2;

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(VBType) * vertex_number_;
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
  FAILTHROW;

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
  FAILTHROW;

  /* =====CB===== */
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBTransformType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_transform_);
  FAILTHROW;

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBCameraLightType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_camera_light_);
  FAILTHROW;

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBMaterialType);
  buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = 0;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  CBMaterialType material;
  material.ka = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);
  material.kd = DirectX::XMFLOAT4(obj.materials[0].diffuse);
  material.kd.w = 1.0f;
  material.ks = DirectX::XMFLOAT4(obj.materials[0].specular);
  material.ks.w = 1.0f;
  material.ns = obj.materials[0].shininess;
  ZeroMemory(&sub_data, sizeof(sub_data));
  sub_data.pSysMem = &material;
  sub_data.SysMemPitch = 0;
  sub_data.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&buffer_desc, &sub_data, &const_buffer_material_);
  FAILTHROW;

  /* =====Texture=====*/
  auto texture_name = name_ + "_stone.dds";
  WCHAR texture_name_l[128] = {0};
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)texture_name.size(), texture_name_l, 128);
  DirectX::CreateDDSTextureFromFile(device, texture_name_l, nullptr,
                                    &shader_resource_texture_);
  FAILTHROW;

  /* =====SamplerState===== */
  sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.MipLODBias = 0.0f;
  sampler_desc.MaxAnisotropy = 1;
  sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  sampler_desc.BorderColor[0] = 0;
  sampler_desc.BorderColor[1] = 0;
  sampler_desc.BorderColor[2] = 0;
  sampler_desc.BorderColor[3] = 0;
  sampler_desc.MinLOD = 0;
  sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = device->CreateSamplerState(&sampler_desc, &sampler_state_);
  FAILTHROW;
}
void Model3D::ShutdownBuffer() {
  SafeRelease(&sampler_state_);
  SafeRelease(&shader_resource_texture_);
  SafeRelease(&const_buffer_material_);
  SafeRelease(&const_buffer_camera_light_);
  SafeRelease(&const_buffer_transform_);
  SafeRelease(&index_buffer_);
  SafeRelease(&vertex_buffer_);
}

void Model3D::InitializeShader(ID3D11Device* device) {
  HRESULT hr = S_OK;

  ID3D10Blob* blob = nullptr;
  const UINT layout_number = 3;
  D3D11_INPUT_ELEMENT_DESC layout[layout_number];

  UINT shader_flag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                    D3DCOMPILE_SKIP_OPTIMIZATION;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"Res/sphere_vs.hlsl", nullptr, nullptr, "VS",
                          "vs_5_0", shader_flag, 0, &blob, nullptr);
  FAILTHROW;
  hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                  blob->GetBufferSize(), nullptr,
                                  &vertex_shader_);
  FAILTHROW;

  ZeroMemory(layout, sizeof(layout));
  layout[0].SemanticName = "POSITION";
  layout[0].SemanticIndex = 0;
  layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  layout[0].InputSlot = 0;
  layout[0].AlignedByteOffset = 0;
  layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  layout[0].InstanceDataStepRate = 0;
  layout[1].SemanticName = "TEXCOORD";
  layout[1].SemanticIndex = 0;
  layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  layout[1].InputSlot = 0;
  layout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  layout[1].InstanceDataStepRate = 0;
  layout[2].SemanticName = "NORMAL";
  layout[2].SemanticIndex = 0;
  layout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  layout[2].InputSlot = 0;
  layout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  layout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  layout[2].InstanceDataStepRate = 0;
  hr = device->CreateInputLayout(layout, layout_number, blob->GetBufferPointer(),
                                 blob->GetBufferSize(), &input_layout_);
  FAILTHROW;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"Res/sphere_ps.hlsl", nullptr, nullptr, "PS",
                          "ps_5_0", shader_flag, 0, &blob, nullptr);
  FAILTHROW;
  hr = device->CreatePixelShader(
      blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixel_shader_);
  FAILTHROW;
  SafeRelease(&blob);
}
void naiive::entity::Model3D::ShutdownShader() {
  SafeRelease(&pixel_shader_);
  SafeRelease(&input_layout_);
  SafeRelease(&vertex_shader_);
}
}  // namespace naiive::entity
