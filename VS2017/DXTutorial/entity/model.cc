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

#include <entity/frustum.h>
#include <utils/range.h>

#ifndef NAIIVE_FRUSTUM_CULL
#define NAIIVE_FRUSTUM_CULL 1
#endif

namespace naiive::entity {
void Model3D::Initialize(ID3D11Device* device) {
  HRESULT hr = S_OK;
  mesh_->Initialize(device);
  shader_->Initialize(device);
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
}

void Model3D::Shutdown() {
  shader_->Shutdown();
  mesh_->Shutdown();
  SafeRelease(&index_buffer_);
  SafeRelease(&vertex_buffer_);
}

BOOL Model3D::Render(ID3D11DeviceContext* context,
                     const DirectX::XMFLOAT4X4& view,
                     const DirectX::XMFLOAT4X4& proj,
                     const DirectX::XMFLOAT4& camera_pos,
                     const DirectX::XMFLOAT4& dir) {
  HRESULT hr = S_OK;

  DirectX::XMFLOAT4X4 world;
  auto xmworld = DirectX::XMMatrixRotationRollPitchYaw(
      rotation_pyr_.x, rotation_pyr_.y, rotation_pyr_.z);
  xmworld = DirectX::XMMatrixMultiply(
      xmworld,
      DirectX::XMMatrixTranslation(translate_.x, translate_.y, translate_.z));
  DirectX::XMStoreFloat4x4(&world, xmworld);
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

  UINT offset = 0U;
  context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride_, &offset);
  context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  shader_->PreRender(context, world, view, proj, camera_pos, dir,
                     mesh_->material(), mesh_->diffuse_map(),
                     mesh_->normal_map());
  shader_->Render(context);
  context->DrawIndexed(mesh_->index_number(), 0, 0);
  return TRUE;
}

}  // namespace naiive::entity
