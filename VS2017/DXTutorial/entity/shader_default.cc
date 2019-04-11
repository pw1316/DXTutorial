#include "shader_default.h"

#include <entity/frustum.h>
#include <entity/model.h>
#include <entity/vertex_type.h>
#include <utils/range.h>

// TODO fix
#ifndef NAIIVE_FRUSTUM_CULL
#define NAIIVE_FRUSTUM_CULL 0
#endif

void naiive::entity::ShaderDefault::Initialize(ID3D11Device* device) {
  InitializeResource(device);
  InitializeShader(device);
}

void naiive::entity::ShaderDefault::Shutdown() {
  ShutdownShader();
  ShutdownResource();
}

BOOL naiive::entity::ShaderDefault::Render(
    ID3D11DeviceContext* context, const Model3D& model,
    const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj,
    const DirectX::XMFLOAT4& camera_pos, const DirectX::XMFLOAT4& light_dir,
    const DirectX::XMFLOAT4& fog, const DirectX::XMFLOAT4& clip_plane) {
  HRESULT hr = S_OK;

  auto xmworld = model.Transform();
  auto xmview = DirectX::XMLoadFloat4x4(&view);
  auto xmproj = DirectX::XMLoadFloat4x4(&proj);
  DirectX::XMFLOAT4X4 matrix_view_proj;
  DirectX::XMStoreFloat4x4(&matrix_view_proj,
                           DirectX::XMMatrixMultiply(xmview, xmproj));
#if NAIIVE_FRUSTUM_CULL
  FrustumWorld frustum(matrix_view_proj);
  BOOL visible = FALSE;
  for (auto&& i : utils::Range(8)) {
    auto&& cnr = model.Corner(i);
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
    rawdata->light_dir = light_dir;
    rawdata->fog = fog;
    rawdata->clip_plane = clip_plane;
  }
  context->Unmap(const_buffer_camera_light_, 0);

  hr = context->Map(const_buffer_material_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                    &mapped);
  ASSERT(SUCCEEDED(hr));
  {
    auto rawdata = (CBMaterialType*)mapped.pData;
    auto&& material = model.Material();
    rawdata->ka = {0.05f, 0.05f, 0.05f, 1.0f};
    rawdata->kd = DirectX::XMFLOAT4(material.diffuse);
    rawdata->kd.w = 1.0f;
    rawdata->ks = DirectX::XMFLOAT4(material.specular);
    rawdata->ks.w = 1.0f;
    rawdata->ns = material.shininess;
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

  context->PSSetConstantBuffers(0, 1, &const_buffer_material_);
  context->PSSetConstantBuffers(1, 1, &const_buffer_camera_light_);
  context->PSSetShaderResources(0, 3, model.ShaderResource());
  context->PSSetSamplers(0, 1, &sampler_state_);
  context->PSSetShader(pixel_shader_, nullptr, 0);

  context->DrawIndexed(model.vertex_number(), 0, 0);
  return TRUE;
}

void naiive::entity::ShaderDefault::InitializeShader(ID3D11Device* device) {
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

void naiive::entity::ShaderDefault::ShutdownShader() {
  SafeRelease(&pixel_shader_);
  SafeRelease(&input_layout_);
  SafeRelease(&vertex_shader_);
}

void naiive::entity::ShaderDefault::InitializeResource(ID3D11Device* device) {
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
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  // CBMaterialType material;
  // material.ka = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);
  // material.kd = DirectX::XMFLOAT4(obj.materials[0].diffuse);
  // material.kd.w = 1.0f;
  // material.ks = DirectX::XMFLOAT4(obj.materials[0].specular);
  // material.ks.w = 1.0f;
  // material.ns = obj.materials[0].shininess;
  // ZeroMemory(&sub_data, sizeof(sub_data));
  // sub_data.pSysMem = &material;
  // sub_data.SysMemPitch = 0;
  // sub_data.SysMemSlicePitch = 0;
  // hr = device->CreateBuffer(&buffer_desc, &sub_data,
  // &const_buffer_material_);
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_material_);
  ASSERT(SUCCEEDED(hr));

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
  ASSERT(SUCCEEDED(hr));
}

void naiive::entity::ShaderDefault::ShutdownResource() {
  SafeRelease(&sampler_state_);
  SafeRelease(&const_buffer_material_);
  SafeRelease(&const_buffer_camera_light_);
  SafeRelease(&const_buffer_transform_);
}
