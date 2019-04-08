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

#include "font.h"

#include <fstream>
#include <vector>

#include <DirectX/DDSTextureLoader.h>

namespace naiive::entity {
void Font::Initialize(ID3D11Device* device) {
  std::ifstream font_meta("Res/font_meta.txt");
  ASSERT(font_meta);

  for (UINT i = 0; i < kNumChar; ++i) {
    int dummy;
    font_meta >> dummy;
    font_meta >> font_[i].uv_left;
    font_meta >> font_[i].uv_right;
    font_meta >> font_[i].size;
  }
  font_meta.close();

  InitializeBuffer(device);
  InitializeShader(device);
}
void Font::Shutdown() {
  ShutdownShader();
  ShutdownBuffer();
}
void Font::Render(ID3D11Device* device, ID3D11DeviceContext* context,
                  const std::string& text, const DirectX::XMFLOAT2& pos,
                  DirectX::XMFLOAT4X4 proj) {
  HRESULT hr = S_OK;

  D3D11_BUFFER_DESC buffer_desc;
  D3D11_SUBRESOURCE_DATA sub_data;
  D3D11_MAPPED_SUBRESOURCE mapped{};

  UINT num_character = static_cast<UINT>(text.size());
  UINT num_vertex = 0U;
  std::vector<VBType> vertices(num_character * 6);
  std::vector<ULONG> indices(num_character * 6);
  FLOAT x = -1.0f / proj._11 + pos.x;
  FLOAT y = 1.0f / proj._22 - pos.y;
  for (auto&& letter : text) {
    ASSERT(letter > 0);
    if (letter == ' ') {
      x += font_[0].size;
    } else if (letter == '\n') {
      x = -1.0f / proj._11 + pos.x;
      y -= 16.0f;
    } else if (letter > kStartChar && letter != 127) {
      UINT index = letter - kStartChar;
      vertices[num_vertex + 0].pos = {x, y, 1.0f};
      vertices[num_vertex + 0].uv = {font_[index].uv_left, 0.0f};
      indices[num_vertex + 0] = num_vertex + 0U;

      vertices[num_vertex + 1].pos = {x + font_[index].size, y - 16, 1.0f};
      vertices[num_vertex + 1].uv = {font_[index].uv_right, 1.0f};
      indices[num_vertex + 1] = num_vertex + 1;

      vertices[num_vertex + 2].pos = {x, y - 16, 1.0f};
      vertices[num_vertex + 2].uv = {font_[index].uv_left, 1.0f};
      indices[num_vertex + 2] = num_vertex + 2;

      vertices[num_vertex + 3].pos = {x, y, 1.0f};
      vertices[num_vertex + 3].uv = {font_[index].uv_left, 0.0f};
      indices[num_vertex + 3] = num_vertex + 3;

      vertices[num_vertex + 4].pos = {x + font_[index].size, y, 1.0f};
      vertices[num_vertex + 4].uv = {font_[index].uv_right, 0.0f};
      indices[num_vertex + 4] = num_vertex + 4;

      vertices[num_vertex + 5].pos = {x + font_[index].size, y - 16, 1.0f};
      vertices[num_vertex + 5].uv = {font_[index].uv_right, 1.0f};
      indices[num_vertex + 5] = num_vertex + 5;
      x += font_[index].size + 1.0f;
      num_vertex += 6;
    }
  }
  vertices.resize(num_vertex);
  indices.resize(num_vertex);

  SafeRelease(&vertex_buffer_);
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
  buffer_desc.ByteWidth = sizeof(VBType) * num_vertex;
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

  SafeRelease(&index_buffer_);
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
  buffer_desc.ByteWidth = sizeof(ULONG) * num_vertex;
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

  context->Map(const_buffer_0_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (CB0Type*)mapped.pData;
    auto temp = DirectX::XMLoadFloat4x4(&proj);
    temp = DirectX::XMMatrixTranspose(temp);
    DirectX::XMStoreFloat4x4(&rawdata->matrix_proj, temp);
    rawdata->font_color = {1, 0, 0, 1};
  }
  context->Unmap(const_buffer_0_, 0);

  UINT stride = sizeof(VBType);
  UINT offset = 0;
  context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
  context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetInputLayout(input_layout_);

  context->VSSetConstantBuffers(0, 1, &const_buffer_0_);
  context->VSSetShader(vertex_shader_, nullptr, 0);

  context->PSSetConstantBuffers(0, 1, &const_buffer_0_);
  context->PSSetShaderResources(0, 1, &shader_resource_font_texture_);
  context->PSSetSamplers(0, 1, &sampler_state_);
  context->PSSetShader(pixel_shader_, nullptr, 0);

  context->DrawIndexed(num_vertex, 0, 0);
}

void Font::InitializeBuffer(ID3D11Device* device) {
  HRESULT hr = S_OK;

  D3D11_BUFFER_DESC buffer_desc;
  D3D11_SAMPLER_DESC sample_desc;

  /* =====VB&IB===== */
  /**
      Byte width is not static, initialize in render
  **/

  /* =====CB===== */
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CB0Type);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_0_);
  ASSERT(SUCCEEDED(hr));

  /* =====Texture===== */
  hr = DirectX::CreateDDSTextureFromFile(device, L"Res/font.dds", nullptr,
                                         &shader_resource_font_texture_);
  ASSERT(SUCCEEDED(hr));

  /* =====SamplerState===== */
  sample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sample_desc.MipLODBias = 0.0f;
  sample_desc.MaxAnisotropy = 1;
  sample_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  sample_desc.BorderColor[0] = 0;
  sample_desc.BorderColor[1] = 0;
  sample_desc.BorderColor[2] = 0;
  sample_desc.BorderColor[3] = 0;
  sample_desc.MinLOD = 0;
  sample_desc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = device->CreateSamplerState(&sample_desc, &sampler_state_);
  ASSERT(SUCCEEDED(hr));
}
void Font::ShutdownBuffer() {
  SafeRelease(&sampler_state_);
  SafeRelease(&shader_resource_font_texture_);
  SafeRelease(&const_buffer_0_);
  SafeRelease(&index_buffer_);
  SafeRelease(&vertex_buffer_);
}

void Font::InitializeShader(ID3D11Device* device) {
  HRESULT hr = S_OK;

  ID3D10Blob* blob = nullptr;
  const UINT layout_number = 2;
  D3D11_INPUT_ELEMENT_DESC layout[layout_number];

  UINT shader_flag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                     D3DCOMPILE_SKIP_OPTIMIZATION;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"res/font_vs.hlsl", nullptr, nullptr, "main",
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
  hr = D3DCompileFromFile(L"res/font_ps.hlsl", nullptr, nullptr, "main",
                          "ps_5_0", shader_flag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreatePixelShader(
      blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixel_shader_);
  ASSERT(SUCCEEDED(hr));
  SafeRelease(&blob);
}
void Font::ShutdownShader() {
  SafeRelease(&pixel_shader_);
  SafeRelease(&input_layout_);
  SafeRelease(&vertex_shader_);
}
}  // namespace naiive::entity
