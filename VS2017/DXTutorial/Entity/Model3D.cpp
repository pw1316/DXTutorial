#include "Model3D.hpp"

#include <locale>
#include <vector>

#include <DirectX/DDSTextureLoader.h>
#include <d3dcompiler.h>

#include <Entity/Frustum.hpp>
#include <Utils/Range.hpp>

void Naiive::Entity::Model3D::Initialize(ID3D11Device* device) {
  InitializeBuffer(device);
  InitializeShader(device);
}

void Naiive::Entity::Model3D::Shutdown() {
  ShutdownShader();
  ShutdownBuffer();
}

BOOL Naiive::Entity::Model3D::Render(ID3D11DeviceContext* context,
                                     DirectX::XMFLOAT4X4 view,
                                     DirectX::XMFLOAT4X4 proj,
                                     DirectX::XMFLOAT4 camPos,
                                     DirectX::XMFLOAT3 dir) {
  HRESULT hr = S_OK;
  D3D11_MAPPED_SUBRESOURCE mapped{};

  auto xmworld = DirectX::XMMatrixRotationRollPitchYawFromVector(
      DirectX::XMLoadFloat3(&m_rotationPYR));
  xmworld = DirectX::XMMatrixMultiply(xmworld,
                                      DirectX::XMMatrixTranslationFromVector(
                                          DirectX::XMLoadFloat3(&m_translate)));
  auto xmview = DirectX::XMLoadFloat4x4(&view);
  auto xmproj = DirectX::XMLoadFloat4x4(&proj);
#if NAIIVE_FRUSTUM_CULL
  FrustumWorld frustum(DirectX::XMMatrixMultiply(xmview, xmproj));
  BOOL visible = FALSE;
  for (auto i : Utils::Range(8)) {
    auto cnr = m_aabb.Corner(i);
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
  context->Map(m_CBTransform, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (CBTransformType*)mapped.pData;

    DirectX::XMStoreFloat4x4(&rawdata->world,
                             DirectX::XMMatrixTranspose(xmworld));
    DirectX::XMStoreFloat4x4(&rawdata->view,
                             DirectX::XMMatrixTranspose(xmview));
    DirectX::XMStoreFloat4x4(&rawdata->proj,
                             DirectX::XMMatrixTranspose(xmproj));
  }
  context->Unmap(m_CBTransform, 0);

  context->Map(m_CBCameraLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  {
    auto rawdata = (CBCameraLightType*)mapped.pData;
    rawdata->camPos = camPos;
    rawdata->lightDir = dir;
  }
  context->Unmap(m_CBCameraLight, 0);

  UINT stride = sizeof(VBType);
  UINT offset = 0;
  context->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
  context->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->IASetInputLayout(m_Layout);

  context->VSSetConstantBuffers(0, 1, &m_CBTransform);
  context->VSSetConstantBuffers(1, 1, &m_CBCameraLight);
  context->VSSetShader(m_VS, nullptr, 0);

  context->PSSetConstantBuffers(0, 1, &m_CBMaterial);
  context->PSSetConstantBuffers(1, 1, &m_CBCameraLight);
  context->PSSetShaderResources(0, 1, &m_SRVTexture0);
  context->PSSetSamplers(0, 1, &m_SamplerState);
  context->PSSetShader(m_PS, nullptr, 0);

  context->DrawIndexed(m_VN, 0, 0);
  return TRUE;
}

void Naiive::Entity::Model3D::InitializeBuffer(ID3D11Device* device) {
  HRESULT hr = S_OK;

  TinyObj obj;
  hr = tinyobj::LoadObj(&obj.attr, &obj.shapes, &obj.materials, nullptr,
                        nullptr, (m_name + ".obj").c_str(), "Res/", true)
           ? S_OK
           : E_FAIL;
  FAILTHROW;

  D3D11_BUFFER_DESC bufferDesc;
  D3D11_SUBRESOURCE_DATA subData;
  D3D11_SAMPLER_DESC sampleDesc;

  /* =====VB & IB===== */
  m_VN = static_cast<UINT>(obj.shapes[0].mesh.indices.size());
  std::vector<VBType> vertices(m_VN);
  std::vector<ULONG> indices(m_VN);
  for (std::remove_const<decltype(m_VN)>::type triId = 0; triId < m_VN / 3;
       ++triId) {
    for (UINT j = 0; j < 3; ++j) {
      auto vi = obj.shapes[0].mesh.indices[3 * triId + 2 - j].vertex_index;
      auto ti = obj.shapes[0].mesh.indices[3 * triId + 2 - j].texcoord_index;
      auto ni = obj.shapes[0].mesh.indices[3 * triId + 2 - j].normal_index;
      auto x = obj.attr.vertices[3 * vi + 0];
      auto y = obj.attr.vertices[3 * vi + 1];
      auto z = obj.attr.vertices[3 * vi + 2];
      m_aabb.Add(DirectX::XMFLOAT3(x, y, -z));
      vertices[3 * triId + j].pos = DirectX::XMFLOAT3(x, y, -z);
      x = obj.attr.texcoords[2 * ti + 0];
      y = obj.attr.texcoords[2 * ti + 1];
      vertices[3 * triId + j].uv = DirectX::XMFLOAT2(x, 1.0f - y);
      x = obj.attr.normals[3 * ni + 0];
      y = obj.attr.normals[3 * ni + 1];
      z = obj.attr.normals[3 * ni + 2];
      vertices[3 * triId + j].normal = DirectX::XMFLOAT3(x, y, -z);
      indices[3 * triId + j] = 3 * triId + j;
    }
  }
  // m_VN = 3;
  // std::vector<VBType> vertices(m_VN);
  // std::vector<ULONG> indices(m_VN);
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

  ZeroMemory(&bufferDesc, sizeof(bufferDesc));
  bufferDesc.ByteWidth = sizeof(VBType) * m_VN;
  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
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

  ZeroMemory(&bufferDesc, sizeof(bufferDesc));
  bufferDesc.ByteWidth = sizeof(ULONG) * m_VN;
  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
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
  bufferDesc.ByteWidth = sizeof(CBCameraLightType);
  bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bufferDesc.MiscFlags = 0;
  bufferDesc.StructureByteStride = 0;
  hr = device->CreateBuffer(&bufferDesc, nullptr, &m_CBCameraLight);
  FAILTHROW;

  ZeroMemory(&bufferDesc, sizeof(bufferDesc));
  bufferDesc.ByteWidth = sizeof(CBMaterialType);
  bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
  bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;
  bufferDesc.StructureByteStride = 0;
  CBMaterialType mt;
  mt.ka = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);
  mt.kd = DirectX::XMFLOAT4(obj.materials[0].diffuse);
  mt.kd.w = 1.0f;
  mt.ks = DirectX::XMFLOAT4(obj.materials[0].specular);
  mt.ks.w = 1.0f;
  mt.ns = obj.materials[0].shininess;
  ZeroMemory(&subData, sizeof(subData));
  subData.pSysMem = &mt;
  subData.SysMemPitch = 0;
  subData.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&bufferDesc, &subData, &m_CBMaterial);
  FAILTHROW;

  /* =====Texture=====*/
  auto texture_name = m_name + "_stone.dds";
  WCHAR texture_name_l[128] = {0};
  MultiByteToWideChar(CP_UTF8, 0, texture_name.c_str(),
                      (int)texture_name.size(), texture_name_l, 128);
  DirectX::CreateDDSTextureFromFile(device, texture_name_l, nullptr,
                                    &m_SRVTexture0);
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
void Naiive::Entity::Model3D::ShutdownBuffer() {
  SafeRelease(&m_SamplerState);
  SafeRelease(&m_SRVTexture0);
  SafeRelease(&m_CBMaterial);
  SafeRelease(&m_CBCameraLight);
  SafeRelease(&m_CBTransform);
  SafeRelease(&m_IB);
  SafeRelease(&m_VB);
}

void Naiive::Entity::Model3D::InitializeShader(ID3D11Device* device) {
  HRESULT hr = S_OK;

  ID3D10Blob* blob = nullptr;
  const UINT nLayout = 3;
  D3D11_INPUT_ELEMENT_DESC layout[nLayout];

  UINT shaderFlag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                    D3DCOMPILE_SKIP_OPTIMIZATION;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"Res/sphere_vs.hlsl", nullptr, nullptr, "VS",
                          "vs_5_0", shaderFlag, 0, &blob, nullptr);
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
  layout[2].SemanticName = "NORMAL";
  layout[2].SemanticIndex = 0;
  layout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  layout[2].InputSlot = 0;
  layout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  layout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  layout[2].InstanceDataStepRate = 0;
  hr = device->CreateInputLayout(layout, nLayout, blob->GetBufferPointer(),
                                 blob->GetBufferSize(), &m_Layout);
  FAILTHROW;

  SafeRelease(&blob);
  hr = D3DCompileFromFile(L"Res/sphere_ps.hlsl", nullptr, nullptr, "PS",
                          "ps_5_0", shaderFlag, 0, &blob, nullptr);
  FAILTHROW;
  hr = device->CreatePixelShader(blob->GetBufferPointer(),
                                 blob->GetBufferSize(), nullptr, &m_PS);
  FAILTHROW;
  SafeRelease(&blob);
}
void Naiive::Entity::Model3D::ShutdownShader() {
  SafeRelease(&m_PS);
  SafeRelease(&m_Layout);
  SafeRelease(&m_VS);
}
