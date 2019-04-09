#include "shader.h"

namespace naiive::entity {
void Shader::Initialize(ID3D11Device* device) {
  InitializeShader(device);
  InitializeResource(device);
}

void Shader::Shutdown() {
  ShutdownResource();
  ShutdownShader();
}

void Shader::PreRender(
    ID3D11DeviceContext* context, const DirectX::XMFLOAT4X4& world,
    const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj,
    const DirectX::XMFLOAT4& camera_pos, const DirectX::XMFLOAT4& dir,
    const tinyobj::material_t& material, ID3D11ShaderResourceView* diffuse_map,
    ID3D11ShaderResourceView* normal_map) {
  HRESULT hr = S_OK;
  D3D11_MAPPED_SUBRESOURCE mapped;
  hr = context->Map(const_buffer_transform_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                    &mapped);
  ASSERT(SUCCEEDED(hr));
  {
    auto rawdata = (CBTransformType*)mapped.pData;
    DirectX::XMStoreFloat4x4(
        &rawdata->world,
        DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&world)));
    DirectX::XMStoreFloat4x4(
        &rawdata->view,
        DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&view)));
    DirectX::XMStoreFloat4x4(
        &rawdata->proj,
        DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&proj)));
  }
  context->Unmap(const_buffer_transform_, 0);
  context->VSSetConstantBuffers(0, 1, &const_buffer_transform_);

  hr = context->Map(const_buffer_aux_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  ASSERT(SUCCEEDED(hr));
  {
    auto rawdata = (CBAuxType*)mapped.pData;
    rawdata->camera_pos = camera_pos;
    rawdata->light_dir = dir;
    rawdata->fog = {0, 50, 0.5f, 0.0f};
    rawdata->clip_plane = {-1, 0, 0, 0};
  }
  context->Unmap(const_buffer_aux_, 0);
  context->VSSetConstantBuffers(1, 1, &const_buffer_aux_);
  context->PSSetConstantBuffers(1, 1, &const_buffer_aux_);

  hr = context->Map(const_buffer_material_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                    &mapped);
  ASSERT(SUCCEEDED(hr));
  {
    auto rawdata = (CBMaterialType*)mapped.pData;
    rawdata->ka = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);
    rawdata->kd = DirectX::XMFLOAT4(material.diffuse);
    rawdata->kd.w = 1.0f;
    rawdata->ks = DirectX::XMFLOAT4(material.specular);
    rawdata->ks.w = 1.0f;
    rawdata->ns = material.shininess;
  }
  context->Unmap(const_buffer_material_, 0);
  context->PSSetConstantBuffers(0, 1, &const_buffer_material_);

  context->PSSetShaderResources(0, 1, &diffuse_map);
  context->PSSetShaderResources(1, 1, &normal_map);
}

BOOL Shader::Render(ID3D11DeviceContext* context) {
  context->IASetInputLayout(input_layout_);
  context->VSSetShader(vertex_shader_, nullptr, 0);
  context->PSSetShader(pixel_shader_, nullptr, 0);
  context->PSSetSamplers(0, 1, &sampler_state_);
  return TRUE;
}

HRESULT STDMETHODCALLTYPE Shader::CreateVertexBufferAndIndexBuffer(
    ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** pp_vertex_buffer,
    ID3D11Buffer** pp_index_buffer, UINT* p_stride) {
  auto index_number = mesh.index_number();
  std::vector<VertexType> vertices(index_number);
  std::vector<ULONG> indices(index_number);
  for (UINT tri_id = 0; tri_id < index_number / 3; ++tri_id) {
    for (UINT v_id = 0; v_id < 3; ++v_id) {
      vertices[3 * tri_id + v_id].pos = mesh.vertex(tri_id, v_id);
      vertices[3 * tri_id + v_id].uv = mesh.texcoord(tri_id, v_id);
      vertices[3 * tri_id + v_id].normal = mesh.normal(tri_id, v_id);
      indices[3 * tri_id + v_id] = 3 * tri_id + v_id;
    }
    /* Calculate T and B */
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

  HRESULT hr = S_OK;
  D3D11_BUFFER_DESC buffer_desc;
  D3D11_SUBRESOURCE_DATA sub_data;
  if (pp_vertex_buffer) {
    ZeroMemory(&buffer_desc, sizeof(buffer_desc));
    buffer_desc.ByteWidth = sizeof(VertexType) * index_number;
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    ZeroMemory(&sub_data, sizeof(sub_data));
    sub_data.pSysMem = vertices.data();
    sub_data.SysMemPitch = 0;
    sub_data.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&buffer_desc, &sub_data, pp_vertex_buffer);
    if (FAILED(hr)) return hr;
  }

  if (pp_index_buffer) {
    ZeroMemory(&buffer_desc, sizeof(buffer_desc));
    buffer_desc.ByteWidth = sizeof(ULONG) * index_number;
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    ZeroMemory(&sub_data, sizeof(sub_data));
    sub_data.pSysMem = indices.data();
    sub_data.SysMemPitch = 0;
    sub_data.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&buffer_desc, &sub_data, pp_index_buffer);
  }

  if (p_stride) {
    *p_stride = sizeof(VertexType);
  }
  return hr;
}

void Shader::InitializeShader(ID3D11Device* device) {
  const UINT kNumLayout = 5;
  const UINT kShaderFlag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG |
                           D3DCOMPILE_SKIP_OPTIMIZATION;
  HRESULT hr = S_OK;
  ID3D10Blob* blob = nullptr;

  std::string shader_name = path_ + "_vs.hlsl";
  WCHAR shader_name_l[128] = {0};
  MultiByteToWideChar(CP_UTF8, 0, shader_name.c_str(),
                      (int)(shader_name.size() + 1), shader_name_l, 128);
  hr = D3DCompileFromFile(shader_name_l, nullptr, nullptr, "main", "vs_5_0",
                          kShaderFlag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                  blob->GetBufferSize(), nullptr,
                                  &vertex_shader_);
  ASSERT(SUCCEEDED(hr));

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
  ASSERT(SUCCEEDED(hr));

  SafeRelease(&blob);
  shader_name = path_ + "_ps.hlsl";
  MultiByteToWideChar(CP_UTF8, 0, shader_name.c_str(),
                      (int)(shader_name.size() + 1), shader_name_l, 128);
  hr = D3DCompileFromFile(shader_name_l, nullptr, nullptr, "main", "ps_5_0",
                          kShaderFlag, 0, &blob, nullptr);
  ASSERT(SUCCEEDED(hr));
  hr = device->CreatePixelShader(
      blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixel_shader_);
  ASSERT(SUCCEEDED(hr));
  SafeRelease(&blob);
}

void Shader::ShutdownShader() {
  SafeRelease(&pixel_shader_);
  SafeRelease(&input_layout_);
  SafeRelease(&vertex_shader_);
}

void Shader::InitializeResource(ID3D11Device* device) {
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
  buffer_desc.ByteWidth = sizeof(CBAuxType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
  hr = device->CreateBuffer(&buffer_desc, nullptr, &const_buffer_aux_);
  ASSERT(SUCCEEDED(hr));

  ZeroMemory(&buffer_desc, sizeof(buffer_desc));
  buffer_desc.ByteWidth = sizeof(CBMaterialType);
  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  buffer_desc.MiscFlags = 0;
  buffer_desc.StructureByteStride = 0;
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

void Shader::ShutdownResource() {
  SafeRelease(&sampler_state_);
  SafeRelease(&const_buffer_material_);
  SafeRelease(&const_buffer_aux_);
  SafeRelease(&const_buffer_transform_);
}
}  // namespace naiive::entity
