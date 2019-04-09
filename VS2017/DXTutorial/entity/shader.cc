#include "shader.h"

namespace naiive::entity {
HRESULT STDMETHODCALLTYPE Shader::CreateVertexBufferAndIndexBuffer(
    ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** pp_vertex_buffer,
    ID3D11Buffer** pp_index_buffer, UINT* p_stride) {
  auto index_number = mesh.index_number();
  std::vector<VBType> vertices(index_number);
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
    buffer_desc.ByteWidth = sizeof(VBType) * index_number;
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

  if (p_stride)
  {
    *p_stride = sizeof(VBType);
  }
  return hr;
}
}  // namespace naiive::entity
