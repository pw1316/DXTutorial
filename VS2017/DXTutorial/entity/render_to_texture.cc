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

#include "render_to_texture.h"

namespace naiive::entity {
void RenderToTexture::Initialize(ID3D11Device* device, UINT width,
                                 UINT height) {
  HRESULT hr = S_OK;

  D3D11_TEXTURE2D_DESC texture2d_desc;
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
  hr = device->CreateTexture2D(&texture2d_desc, nullptr, &back_buffer_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 texture failed", std::hex, std::uppercase, hr);

  hr = device->CreateRenderTargetView(back_buffer_, nullptr,
                                      &render_target_view_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 render target view failed", std::hex, std::uppercase, hr);
  hr = device->CreateShaderResourceView(back_buffer_, nullptr,
                                        &shader_resource_view_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 shader resource view failed", std::hex, std::uppercase, hr);

  ULONG indices[] = {0, 1, 2, 3, 4, 5};

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
  ("Get DirectX 11 const buffer failed", std::hex, std::uppercase, hr);

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.ByteWidth = sizeof(VBType) * 6;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &vertex_buffer_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get DirectX 11 vertex buffer failed", std::hex, std::uppercase, hr);

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
  ("Get DirectX 11 index buffer failed", std::hex, std::uppercase, hr);

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
  ("Get DirectX 11 sampler state failed", std::hex, std::uppercase, hr);

  InitializeShader(device);
}

void RenderToTexture::Shutdown() {
  ShutdownShader();
  SafeRelease(&sampler_state_);
  SafeRelease(&index_buffer_);
  SafeRelease(&vertex_buffer_);
  SafeRelease(&const_buffer_);

  SafeRelease(&shader_resource_view_);
  SafeRelease(&render_target_view_);
  SafeRelease(&back_buffer_);
}

void RenderToTexture::Render(ID3D11DeviceContext* context,
                             const DirectX::XMFLOAT2& pos,
                             DirectX::XMFLOAT4X4 proj) {
  HRESULT hr = S_OK;

  FLOAT x = -1.0f / proj._11 + pos.x;
  FLOAT y = 1.0f / proj._22 - pos.y;

  D3D11_MAPPED_SUBRESOURCE mapped;
  context->Map(const_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (CB0Type*)mapped.pData;
    auto temp = DirectX::XMLoadFloat4x4(&proj);
    temp = DirectX::XMMatrixTranspose(temp);
    DirectX::XMStoreFloat4x4(&rawdata->matrix_proj, temp);
  }
  context->Unmap(const_buffer_, 0);

  context->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (VBType*)mapped.pData;
    rawdata[0].pos = {x, y, 1.0f};
    rawdata[0].uv = {0.0f, 0.0f};
    rawdata[1].pos = {x + kWidth, y - kHeight, 1.0f};
    rawdata[1].uv = {1.0f, 1.0f};
    rawdata[2].pos = {x, y - kHeight, 1.0f};
    rawdata[2].uv = {0.0f, 1.0f};
    rawdata[3].pos = {x, y, 1.0f};
    rawdata[3].uv = {0.0f, 0.0f};
    rawdata[4].pos = {x + kWidth, y, 1.0f};
    rawdata[4].uv = {1.0f, 0.0f};
    rawdata[5].pos = {x + kWidth, y - kHeight, 1.0f};
    rawdata[5].uv = {1.0f, 1.0f};
  }
  context->Unmap(vertex_buffer_, 0);

  UINT stride = sizeof(VBType);
  UINT offset = 0;
  context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
  context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetInputLayout(input_layout_);

  context->VSSetConstantBuffers(0, 1, &const_buffer_);
  context->VSSetShader(vertex_shader_, nullptr, 0);

  context->PSSetConstantBuffers(0, 1, &const_buffer_);
  context->PSSetShaderResources(0, 1, &shader_resource_view_);
  context->PSSetSamplers(0, 1, &sampler_state_);
  context->PSSetShader(pixel_shader_, nullptr, 0);

  context->DrawIndexed(6, 0, 0);
  context->PSSetShaderResources(0, 1, &kDummyShaderResourceView);
}

void RenderToTexture::InitializeShader(ID3D11Device* device) {
  HRESULT hr = S_OK;

  ID3D10Blob* blob = nullptr;
  const UINT layout_number = 2;
  D3D11_INPUT_ELEMENT_DESC layout[layout_number];

  UINT shader_flag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                     D3DCOMPILE_SKIP_OPTIMIZATION;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"res/tex_vs.hlsl", nullptr, nullptr, "main",
                          "vs_5_0", shader_flag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                  blob->GetBufferSize(), nullptr,
                                  &vertex_shader_);
  ASSERT(SUCCEEDED(hr));

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
  hr =
      device->CreateInputLayout(layout, layout_number, blob->GetBufferPointer(),
                                blob->GetBufferSize(), &input_layout_);
  ASSERT(SUCCEEDED(hr));

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"res/tex_ps.hlsl", nullptr, nullptr, "main",
                          "ps_5_0", shader_flag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreatePixelShader(
      blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixel_shader_);
  ASSERT(SUCCEEDED(hr));
  SafeRelease(&blob);
}
void RenderToTexture::ShutdownShader() {
  SafeRelease(&pixel_shader_);
  SafeRelease(&input_layout_);
  SafeRelease(&vertex_shader_);
}
}  // namespace naiive::entity
