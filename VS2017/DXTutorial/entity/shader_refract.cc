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

#include "shader_refract.h"

#include <entity/frustum.h>
#include <entity/model_default.h>
#include <entity/vertex_type.h>
#include <utils/range.h>

namespace naiive::entity {
BOOL ShaderRefract::Render(
    ID3D11DeviceContext* context, const ModelDefault& model,
    const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj,
    const DirectX::XMFLOAT4& camera_pos, const DirectX::XMFLOAT4& light_dir,
    const DirectX::XMFLOAT4& fog, const DirectX::XMFLOAT4& refract_plane) {
  HRESULT hr = S_OK;

  auto xmworld = model.Transform();
  auto xmview = DirectX::XMLoadFloat4x4(&view);
  auto xmproj = DirectX::XMLoadFloat4x4(&proj);
  DirectX::XMFLOAT4X4 matrix_view_proj;
  DirectX::XMStoreFloat4x4(&matrix_view_proj,
                           DirectX::XMMatrixMultiply(xmview, xmproj));
  if (!model.Visible(matrix_view_proj)) {
    return FALSE;
  }

  D3D11_MAPPED_SUBRESOURCE mapped;
  hr = context->Map(const_buffer_transform_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                    &mapped);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Map CB0 failed");
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
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Map CB1 failed");
  {
    auto rawdata = (CBCameraLightType*)mapped.pData;
    rawdata->camera_pos = camera_pos;
    rawdata->light_dir = light_dir;
    rawdata->fog = fog;
    rawdata->refract_plane = refract_plane;
  }
  context->Unmap(const_buffer_camera_light_, 0);

  hr = context->Map(const_buffer_material_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                    &mapped);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Map CB2 failed");
  {
    auto rawdata = (CBMaterialType*)mapped.pData;
    auto&& material = model.Material();
    rawdata->ka = DirectX::XMFLOAT4(material.ambient);
    rawdata->ka.w = 1.0f;
    rawdata->kd = DirectX::XMFLOAT4(material.diffuse);
    rawdata->kd.w = 1.0f;
    rawdata->ks = DirectX::XMFLOAT4(material.specular);
    rawdata->ks.w = 1.0f;
    rawdata->ns.x = material.shininess;
  }
  context->Unmap(const_buffer_material_, 0);

  UINT stride = sizeof(VertexType);
  UINT offset = 0U;
  auto vertex_buffer = model.VertexBuffer();
  context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
  context->IASetIndexBuffer(model.IndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetInputLayout(input_layout_);

  context->VSSetConstantBuffers(0, 1, &const_buffer_transform_);
  context->VSSetConstantBuffers(1, 1, &const_buffer_camera_light_);
  context->VSSetShader(vertex_shader_, nullptr, 0);

  auto srv_diffuse_map = model.DiffuseMap();
  auto srv_bump_map = model.BumpMap();
  context->PSSetConstantBuffers(0, 1, &const_buffer_material_);
  context->PSSetConstantBuffers(1, 1, &const_buffer_camera_light_);
  context->PSSetShaderResources(0, 1, &srv_diffuse_map);
  context->PSSetShaderResources(1, 1, &srv_bump_map);
  context->PSSetSamplers(0, 1, &sampler_state_);
  context->PSSetShader(pixel_shader_, nullptr, 0);

  context->DrawIndexed(model.NumIndices(), 0, 0);
  return TRUE;
}

void ShaderRefract::InitializeShader(ID3D11Device* device) {
  HRESULT hr = S_OK;

  ID3D10Blob* blob = nullptr;
  UINT shader_flag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                     D3DCOMPILE_SKIP_OPTIMIZATION;

  auto shader_name = raw_path_ + "_vs.hlsl";
  WCHAR shader_name_l[128] = {0};
  MultiByteToWideChar(CP_UTF8, 0, shader_name.c_str(),
                      (int)(shader_name.size() + 1), shader_name_l, 128);
  hr = D3DCompileFromFile(shader_name_l, nullptr, nullptr, "main", "vs_5_0",
                          shader_flag, 0, &blob, nullptr);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Compile VS failed");
  hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                  blob->GetBufferSize(), nullptr,
                                  &vertex_shader_);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get VS failed");

  const UINT kNumLayout = 5;
  D3D11_INPUT_ELEMENT_DESC layout[kNumLayout];
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
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get Layout failed");

  SafeRelease(&blob);
  shader_name = raw_path_ + "_ps.hlsl";
  MultiByteToWideChar(CP_UTF8, 0, shader_name.c_str(),
                      (int)(shader_name.size() + 1), shader_name_l, 128);
  hr = D3DCompileFromFile(shader_name_l, nullptr, nullptr, "main", "ps_5_0",
                          shader_flag, 0, &blob, nullptr);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Compile PS failed");
  hr = device->CreatePixelShader(
      blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixel_shader_);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get PS failed");
  SafeRelease(&blob);
}

void ShaderRefract::ShutdownShader() {
  SafeRelease(&pixel_shader_);
  SafeRelease(&input_layout_);
  SafeRelease(&vertex_shader_);
}

void ShaderRefract::InitializeResource(ID3D11Device* device) {
  HRESULT hr = S_OK;

  D3D11_BUFFER_DESC buffer_desc;
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBTransformType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_transform_);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get CB0 failed");

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBCameraLightType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_camera_light_);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get CB1 failed");

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBMaterialType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_material_);
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get CB2 failed");

  D3D11_SAMPLER_DESC sampler_desc;
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
  ASSERT_MESSAGE(SUCCEEDED(hr))(raw_path_, "Get SamplerState failed");
}

void ShaderRefract::ShutdownResource() {
  SafeRelease(&sampler_state_);
  SafeRelease(&const_buffer_material_);
  SafeRelease(&const_buffer_camera_light_);
  SafeRelease(&const_buffer_transform_);
}
}  // namespace naiive::entity
