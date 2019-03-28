#include "Font.hpp"

#include <fstream>
#include <vector>

#include <d3dcompiler.h>

#include <DirectX/DDSTextureLoader.h>

void naiive::Entity::Font::Initialize(ID3D11Device* device) {
  HRESULT hr = S_OK;
  std::ifstream fontMeta("Res/font_meta.txt");
  hr = fontMeta ? S_OK : E_FAIL;
  FAILTHROW;

  for (int i = 0; i < 95; ++i) {
    int dummy;
    fontMeta >> dummy;
    fontMeta >> m_Font[i].left;
    fontMeta >> m_Font[i].right;
    fontMeta >> m_Font[i].size;
  }
  fontMeta.close();

  InitializeBuffer(device);
  InitializeShader(device);
}
void naiive::Entity::Font::Shutdown() {
  ShutdownShader();
  ShutdownBuffer();
}
void naiive::Entity::Font::Render(ID3D11Device* device,
                                  ID3D11DeviceContext* context,
                                  const std::string& text,
                                  const DirectX::XMFLOAT2& pos,
                                  DirectX::XMFLOAT4X4 proj) {
  HRESULT hr = S_OK;

  D3D11_BUFFER_DESC bufferDesc;
  D3D11_SUBRESOURCE_DATA subData;
  D3D11_MAPPED_SUBRESOURCE mapped{};

  UINT VN = static_cast<UINT>(text.size() * 6);
  UINT RVN = 0U;
  std::vector<VBType> vertices(VN);
  std::vector<ULONG> indices(VN);
  float x = -512 + pos.x;
  float y = 384 - pos.y;
  for (UINT vId = 0; vId < VN / 6; ++vId) {
    auto letter = text[vId] - 32;
    if (letter == 0) {
    } else if (letter == -22) {
      x = -512 + pos.x;
      y -= 16.0f;
    } else {
      vertices[RVN + 0].pos = DirectX::XMFLOAT3(x, y, 1.0f);
      vertices[RVN + 0].uv = DirectX::XMFLOAT2(m_Font[letter].left, 0.0f);
      indices[RVN + 0] = RVN + 0;

      vertices[RVN + 1].pos =
          DirectX::XMFLOAT3((x + m_Font[letter].size), (y - 16), 1.0f);
      vertices[RVN + 1].uv = DirectX::XMFLOAT2(m_Font[letter].right, 1.0f);
      indices[RVN + 1] = RVN + 1;

      vertices[RVN + 2].pos = DirectX::XMFLOAT3(x, (y - 16), 1.0f);
      vertices[RVN + 2].uv = DirectX::XMFLOAT2(m_Font[letter].left, 1.0f);
      indices[RVN + 2] = RVN + 2;

      vertices[RVN + 3].pos = DirectX::XMFLOAT3(x, y, 1.0f);
      vertices[RVN + 3].uv = DirectX::XMFLOAT2(m_Font[letter].left, 0.0f);
      indices[RVN + 3] = RVN + 3;

      vertices[RVN + 4].pos =
          DirectX::XMFLOAT3(x + m_Font[letter].size, y, 1.0f);
      vertices[RVN + 4].uv = DirectX::XMFLOAT2(m_Font[letter].right, 0.0f);
      indices[RVN + 4] = RVN + 4;

      vertices[RVN + 5].pos =
          DirectX::XMFLOAT3((x + m_Font[letter].size), (y - 16), 1.0f);
      vertices[RVN + 5].uv = DirectX::XMFLOAT2(m_Font[letter].right, 1.0f);
      indices[RVN + 5] = RVN + 5;
      x += m_Font[letter].size + 1.0f;
      RVN += 6;
    }
  }
  vertices.resize(RVN);
  indices.resize(RVN);

  SafeRelease(&m_VB);
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));
  bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
  bufferDesc.ByteWidth = sizeof(VBType) * RVN;
  bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;
  bufferDesc.StructureByteStride = 0;
  ZeroMemory(&subData, sizeof(subData));
  subData.pSysMem = vertices.data();
  subData.SysMemPitch = 0;
  subData.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&bufferDesc, &subData, &m_VB);
  FAILTHROW;

  SafeRelease(&m_IB);
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));
  bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
  bufferDesc.ByteWidth = sizeof(ULONG) * RVN;
  bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;
  bufferDesc.StructureByteStride = 0;
  ZeroMemory(&subData, sizeof(subData));
  subData.pSysMem = indices.data();
  subData.SysMemPitch = 0;
  subData.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&bufferDesc, &subData, &m_IB);
  FAILTHROW;

  context->Map(m_CBTransform, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (CBTransformType*)mapped.pData;
    DirectX::XMMATRIX temp = DirectX::XMLoadFloat4x4(&proj);
    temp = DirectX::XMMatrixTranspose(temp);
    DirectX::XMStoreFloat4x4(&rawdata->proj, temp);
  }
  context->Unmap(m_CBTransform, 0);

  context->Map(m_CBColor, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (CBColorType*)mapped.pData;
    rawdata->color = DirectX::XMFLOAT4(1, 0, 0, 1);
  }
  context->Unmap(m_CBColor, 0);

  UINT stride = sizeof(VBType);
  UINT offset = 0;
  context->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
  context->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetInputLayout(m_Layout);

  context->VSSetConstantBuffers(0, 1, &m_CBTransform);
  context->VSSetShader(m_VS, nullptr, 0);

  context->PSSetConstantBuffers(0, 1, &m_CBColor);
  context->PSSetShaderResources(0, 1, &m_SRVTexture);
  context->PSSetSamplers(0, 1, &m_SamplerState);
  context->PSSetShader(m_PS, nullptr, 0);

  context->DrawIndexed(RVN, 0, 0);
}

void naiive::Entity::Font::InitializeBuffer(ID3D11Device* device) {
  HRESULT hr = S_OK;

  D3D11_BUFFER_DESC bufferDesc;
  D3D11_SAMPLER_DESC sampleDesc;

  /* =====VB&IB===== */
  /**
      Byte width is not static, initialize in render
  **/

  /* =====CB===== */
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));
  bufferDesc.ByteWidth = sizeof(CBTransformType);
  bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bufferDesc.MiscFlags = 0;
  bufferDesc.StructureByteStride = 0;
  hr = device->CreateBuffer(&bufferDesc, nullptr, &m_CBTransform);
  FAILTHROW;

  ZeroMemory(&bufferDesc, sizeof(bufferDesc));
  bufferDesc.ByteWidth = sizeof(CBColorType);
  bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bufferDesc.MiscFlags = 0;
  bufferDesc.StructureByteStride = 0;
  hr = device->CreateBuffer(&bufferDesc, nullptr, &m_CBColor);
  FAILTHROW;

  /* =====Texture===== */
  ID3D11Resource* resource = nullptr;
  hr = DirectX::CreateDDSTextureFromFile(device, L"Res/font.dds", nullptr,
                                         &m_SRVTexture);
  FAILTHROW;

  /* =====SamplerState===== */
  sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampleDesc.MipLODBias = 0.0f;
  sampleDesc.MaxAnisotropy = 1;
  sampleDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  sampleDesc.BorderColor[0] = 0;
  sampleDesc.BorderColor[1] = 0;
  sampleDesc.BorderColor[2] = 0;
  sampleDesc.BorderColor[3] = 0;
  sampleDesc.MinLOD = 0;
  sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = device->CreateSamplerState(&sampleDesc, &m_SamplerState);
  FAILTHROW;
}
void naiive::Entity::Font::ShutdownBuffer() {
  SafeRelease(&m_SamplerState);
  SafeRelease(&m_SRVTexture);
  SafeRelease(&m_CBColor);
  SafeRelease(&m_CBTransform);
  SafeRelease(&m_IB);
  SafeRelease(&m_VB);
}

void naiive::Entity::Font::InitializeShader(ID3D11Device* device) {
  HRESULT hr = S_OK;

  ID3D10Blob* blob = nullptr;
  const UINT nLayout = 2;
  D3D11_INPUT_ELEMENT_DESC layout[nLayout];

  UINT shaderFlag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                    D3DCOMPILE_SKIP_OPTIMIZATION;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"Res/font_vs.hlsl", nullptr, nullptr, "VS", "vs_5_0",
                          shaderFlag, 0, &blob, nullptr);
  FAILTHROW;
  hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                  blob->GetBufferSize(), nullptr, &m_VS);
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
  hr = device->CreateInputLayout(layout, nLayout, blob->GetBufferPointer(),
                                 blob->GetBufferSize(), &m_Layout);
  FAILTHROW;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"Res/font_ps.hlsl", nullptr, nullptr, "PS", "ps_5_0",
                          shaderFlag, 0, &blob, nullptr);
  FAILTHROW;
  hr = device->CreatePixelShader(blob->GetBufferPointer(),
                                 blob->GetBufferSize(), nullptr, &m_PS);
  FAILTHROW;
  SafeRelease(&blob);
}
void naiive::Entity::Font::ShutdownShader() {
  SafeRelease(&m_PS);
  SafeRelease(&m_Layout);
  SafeRelease(&m_VS);
}
