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

#include "mirror.h"

#include <entity/vertex_type.h>

namespace naiive::entity {
void Mirror::Initialize(ID3D11Device* device, UINT width, UINT height) {
  HRESULT hr = S_OK;

  D3D11_TEXTURE2D_DESC texture2d_desc;
  // reflect buffer
  ZeroMemory(&texture2d_desc, sizeof(texture2d_desc));
  texture2d_desc.Width = width;
  texture2d_desc.Height = height;
  texture2d_desc.MipLevels = 1;
  texture2d_desc.ArraySize = 1;
  texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  texture2d_desc.SampleDesc.Count = 1;
  texture2d_desc.SampleDesc.Quality = 0;
  texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
  texture2d_desc.BindFlags =
      D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  texture2d_desc.CPUAccessFlags = 0;
  texture2d_desc.MiscFlags = 0;
  hr = device->CreateTexture2D(&texture2d_desc, nullptr, &texture_reflect_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 Texture2D failed", STD_HEX(hr));
  hr = device->CreateRenderTargetView(texture_reflect_, nullptr, &rtv_reflect_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 RTV failed", STD_HEX(hr));
  hr = device->CreateShaderResourceView(texture_reflect_, nullptr,
                                        &srv_reflect_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 SRV failed", STD_HEX(hr));

  // refract buffer
  hr = device->CreateTexture2D(&texture2d_desc, nullptr, &texture_refract_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 Texture2D failed", STD_HEX(hr));
  hr = device->CreateRenderTargetView(texture_refract_, nullptr, &rtv_refract_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 RTV failed", STD_HEX(hr));
  hr = device->CreateShaderResourceView(texture_refract_, nullptr,
                                        &srv_refract_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 SRV failed", STD_HEX(hr));

  D3D11_BUFFER_DESC buffer_desc;
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.ByteWidth = sizeof(CB0Type);
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 CB failed", STD_HEX(hr));

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.ByteWidth = sizeof(VertexType) * 6;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &vertex_buffer_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 VB failed", STD_HEX(hr));

  ULONG indices[] = {0, 1, 2, 3, 4, 5};
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
  buffer_desc.ByteWidth = sizeof(ULONG) * 6;
  buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  buffer_desc.CPUAccessFlags = 0;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  D3D11_SUBRESOURCE_DATA subdata;
  ZeroMemory(&subdata, sizeof(subdata));
  subdata.pSysMem = indices;
  subdata.SysMemPitch = 0;
  subdata.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&buffer_desc, &subdata, &index_buffer_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 IB failed", STD_HEX(hr));

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
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 SamplerState failed", STD_HEX(hr));

  InitializeShader(device);
}

void Mirror::Shutdown() {
  ShutdownShader();
  SafeRelease(&sampler_state_);
  SafeRelease(&index_buffer_);
  SafeRelease(&vertex_buffer_);
  SafeRelease(&const_buffer_);

  SafeRelease(&srv_refract_);
  SafeRelease(&rtv_refract_);
  SafeRelease(&texture_refract_);
  SafeRelease(&srv_reflect_);
  SafeRelease(&rtv_reflect_);
  SafeRelease(&texture_reflect_);
}

void Mirror::Render(ID3D11DeviceContext* context,
                    const DirectX::XMFLOAT4X4& view,
                    const DirectX::XMFLOAT4X4& proj,
                    const DirectX::XMFLOAT4X4& view_reflect) {
  HRESULT hr = S_OK;

  D3D11_MAPPED_SUBRESOURCE mapped;
  context->Map(const_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (CB0Type*)mapped.pData;
    auto temp = DirectX::XMMatrixIdentity();
    DirectX::XMStoreFloat4x4(&rawdata->matrix_world, temp);

    temp = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&view));
    DirectX::XMStoreFloat4x4(&rawdata->matrix_view, temp);

    temp = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&proj));
    DirectX::XMStoreFloat4x4(&rawdata->matrix_proj, temp);

    temp = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&view_reflect));
    DirectX::XMStoreFloat4x4(&rawdata->matrix_reflect_view, temp);
  }
  context->Unmap(const_buffer_, 0);

  context->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (VertexType*)mapped.pData;
    rawdata[0].pos = {-80, -5, 80, 1};
    rawdata[0].uv = {0.0f, 0.0f};
    rawdata[1].pos = {80, -5, -80, 1};
    rawdata[1].uv = {1.0f, 1.0f};
    rawdata[2].pos = {-80, -5, -80, 1};
    rawdata[2].uv = {0.0f, 1.0f};
    rawdata[3].pos = {-80, -5, 80, 1};
    rawdata[3].uv = {0.0f, 0.0f};
    rawdata[4].pos = {80, -5, 80, 1};
    rawdata[4].uv = {1.0f, 0.0f};
    rawdata[5].pos = {80, -5, -80, 1};
    rawdata[5].uv = {1.0f, 1.0f};
  }
  context->Unmap(vertex_buffer_, 0);

  UINT stride = sizeof(VertexType);
  UINT offset = 0;
  context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
  context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetInputLayout(input_layout_);

  context->VSSetConstantBuffers(0, 1, &const_buffer_);
  context->VSSetShader(vertex_shader_, nullptr, 0);

  context->PSSetShaderResources(0, 1, &srv_reflect_);
  context->PSSetShaderResources(1, 1, &srv_refract_);
  context->PSSetSamplers(0, 1, &sampler_state_);
  context->PSSetShader(pixel_shader_, nullptr, 0);

  context->DrawIndexed(6, 0, 0);
  context->PSSetShaderResources(0, 1, &kDummyShaderResourceView);
  context->PSSetShaderResources(1, 1, &kDummyShaderResourceView);
}

void Mirror::InitializeShader(ID3D11Device* device) {
  HRESULT hr = S_OK;

  ID3D10Blob* blob = nullptr;
  UINT shader_flag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                     D3DCOMPILE_SKIP_OPTIMIZATION;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"res/water_vs.hlsl", nullptr, nullptr, "main",
                          "vs_5_0", shader_flag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                  blob->GetBufferSize(), nullptr,
                                  &vertex_shader_);
  ASSERT(SUCCEEDED(hr));

  const UINT kNumLayout = 1;
  D3D11_INPUT_ELEMENT_DESC layout[kNumLayout];
  ZeroMemory(layout, sizeof(layout));
  layout[0].SemanticName = "POSITION";
  layout[0].SemanticIndex = 0;
  layout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  layout[0].InputSlot = 0;
  layout[0].AlignedByteOffset = offsetof(VertexType, pos);
  layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  layout[0].InstanceDataStepRate = 0;
  hr = device->CreateInputLayout(layout, kNumLayout, blob->GetBufferPointer(),
                                 blob->GetBufferSize(), &input_layout_);
  ASSERT(SUCCEEDED(hr));

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"res/water_ps.hlsl", nullptr, nullptr, "main",
                          "ps_5_0", shader_flag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreatePixelShader(
      blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixel_shader_);
  ASSERT(SUCCEEDED(hr));
  SafeRelease(&blob);
}
void Mirror::ShutdownShader() {
  SafeRelease(&pixel_shader_);
  SafeRelease(&input_layout_);
  SafeRelease(&vertex_shader_);
}
}  // namespace naiive::entity
