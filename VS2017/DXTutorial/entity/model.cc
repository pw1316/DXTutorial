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
                     const DirectX::XMFLOAT4& dir) {
  HRESULT hr = S_OK;

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
  for (auto&& i : utils::Range(8)) {
    auto&& cnr = aabb_.Corner(i);
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

  D3D11_MAPPED_SUBRESOURCE mapped;
  hr = context->Map(const_buffer_transform_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                    &mapped);
  ASSERT(SUCCEEDED(hr));
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

  hr = context->Map(const_buffer_camera_light_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                    &mapped);
  ASSERT(SUCCEEDED(hr));
  {
    auto rawdata = (CBCameraLightType*)mapped.pData;
    rawdata->camera_pos = camera_pos;
    rawdata->light_dir = dir;
    rawdata->fog = {0, 50, 0.5f, 0.0f};
    rawdata->clip_plane = {-1, 0, 0, 0};
  }
  context->Unmap(const_buffer_camera_light_, 0);

  UINT offset = 0U;
  context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride_, &offset);
  context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetInputLayout(input_layout_);

  context->VSSetConstantBuffers(0, 1, &const_buffer_transform_);
  context->VSSetConstantBuffers(1, 1, &const_buffer_camera_light_);
  context->VSSetShader(vertex_shader_, nullptr, 0);

  context->PSSetConstantBuffers(0, 1, &const_buffer_material_);
  context->PSSetConstantBuffers(1, 1, &const_buffer_camera_light_);
  context->PSSetShaderResources(0, 3, shader_resource_texture_);
  context->PSSetSamplers(0, 1, &sampler_state_);
  context->PSSetShader(pixel_shader_, nullptr, 0);

  context->DrawIndexed(mesh_->index_number(), 0, 0);
  return TRUE;
}

void Model3D::InitializeBuffer(ID3D11Device* device) {
  BOOL bool_res = FALSE;
  HRESULT hr = S_OK;

  D3D11_BUFFER_DESC buffer_desc;
  D3D11_SUBRESOURCE_DATA sub_data;
  D3D11_SAMPLER_DESC sampler_desc;

  /* =====VB & IB===== */
  hr = shader_->CreateVertexBufferAndIndexBuffer(
      device, *mesh_, &vertex_buffer_, &index_buffer_, &stride_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 vertex/index buffer failed", STD_HEX(hr));
  auto index_number = mesh_->index_number();
  for (UINT tri_id = 0; tri_id < index_number / 3; ++tri_id) {
    for (UINT v_id = 0; v_id < 3; ++v_id) {
      auto vertex = mesh_->vertex(tri_id, v_id);
      aabb_.Add(vertex.x, vertex.y, vertex.z);
    }
  }

  /* =====CB===== */
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBTransformType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_transform_);
  ASSERT(SUCCEEDED(hr));

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBCameraLightType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_camera_light_);
  ASSERT(SUCCEEDED(hr));

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBMaterialType);
  buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = 0;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  CBMaterialType material;
  material.ka = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);
  material.kd = DirectX::XMFLOAT4(mesh_->material().diffuse);
  material.kd.w = 1.0f;
  material.ks = DirectX::XMFLOAT4(mesh_->material().specular);
  material.ks.w = 1.0f;
  material.ns = mesh_->material().shininess;
  ZeroMemory(&sub_data, sizeof(sub_data));
  sub_data.pSysMem = &material;
  sub_data.SysMemPitch = 0;
  sub_data.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&buffer_desc, &sub_data, &const_buffer_material_);
  ASSERT(SUCCEEDED(hr));

  /* =====Texture=====*/
  auto texture_name = mesh_->get_path() + "_stone.dds";
  WCHAR texture_name_l[128] = {0};
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)(texture_name.size() + 1), texture_name_l, 128);
  hr = DirectX::CreateDDSTextureFromFileEx(
      device, nullptr, texture_name_l, 0, D3D11_USAGE_DEFAULT,
      D3D11_BIND_SHADER_RESOURCE, 0, 0, TRUE, nullptr,
      &shader_resource_texture_[0]);
  CHECK(SUCCEEDED(hr))("Texture [stone] missing");

  texture_name = mesh_->get_path() + "_stone_bump.dds";
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)(texture_name.size() + 1), texture_name_l, 128);
  hr = DirectX::CreateDDSTextureFromFile(device, texture_name_l, nullptr,
                                         &shader_resource_texture_[1]);
  CHECK(SUCCEEDED(hr))("Texture [stone_bump] missing");

  texture_name = mesh_->get_path() + "_dirt.dds";
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)(texture_name.size() + 1), texture_name_l, 128);
  hr = DirectX::CreateDDSTextureFromFileEx(
      device, nullptr, texture_name_l, 0, D3D11_USAGE_DEFAULT,
      D3D11_BIND_SHADER_RESOURCE, 0, 0, TRUE, nullptr,
      &shader_resource_texture_[2]);
  CHECK(SUCCEEDED(hr))("Texture [dirt] missing");

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
  ASSERT(SUCCEEDED(hr));
}
void Model3D::ShutdownBuffer() {
  SafeRelease(&sampler_state_);
  SafeRelease(&shader_resource_texture_[2]);
  SafeRelease(&shader_resource_texture_[1]);
  SafeRelease(&shader_resource_texture_[0]);
  SafeRelease(&const_buffer_material_);
  SafeRelease(&const_buffer_camera_light_);
  SafeRelease(&const_buffer_transform_);
  SafeRelease(&index_buffer_);
  SafeRelease(&vertex_buffer_);
}

void Model3D::InitializeShader(ID3D11Device* device) {
  HRESULT hr = S_OK;

  ID3D10Blob* blob = nullptr;
  const UINT kNumLayout = 5;
  D3D11_INPUT_ELEMENT_DESC layout[kNumLayout];

  UINT shader_flag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                     D3DCOMPILE_SKIP_OPTIMIZATION;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"res/sphere_vs.hlsl", nullptr, nullptr, "main",
                          "vs_5_0", shader_flag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                  blob->GetBufferSize(), nullptr,
                                  &vertex_shader_);
  ASSERT(SUCCEEDED(hr));

  ZeroMemory(layout, sizeof(layout));
  layout[0].SemanticName = "POSITION";
  layout[0].SemanticIndex = 0;
  layout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
  layout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  layout[2].InputSlot = 0;
  layout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  layout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  layout[2].InstanceDataStepRate = 0;

  layout[3].SemanticName = "TANGENT";
  layout[3].SemanticIndex = 0;
  layout[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  layout[3].InputSlot = 0;
  layout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  layout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  layout[3].InstanceDataStepRate = 0;

  layout[4].SemanticName = "BINORMAL";
  layout[4].SemanticIndex = 0;
  layout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  layout[4].InputSlot = 0;
  layout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  layout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  layout[4].InstanceDataStepRate = 0;
  hr = device->CreateInputLayout(layout, kNumLayout, blob->GetBufferPointer(),
                                 blob->GetBufferSize(), &input_layout_);
  ASSERT(SUCCEEDED(hr));

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"res/sphere_ps.hlsl", nullptr, nullptr, "main",
                          "ps_5_0", shader_flag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreatePixelShader(
      blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixel_shader_);
  ASSERT(SUCCEEDED(hr));
  SafeRelease(&blob);
}
void Model3D::ShutdownShader() {
  SafeRelease(&pixel_shader_);
  SafeRelease(&input_layout_);
  SafeRelease(&vertex_shader_);
}
}  // namespace naiive::entity
