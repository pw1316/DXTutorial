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

#ifndef __ENTITY_MODEL_DEFAULT__
#define __ENTITY_MODEL_DEFAULT__
#include <string>
#include <vector>

#include <3rdparty/include/tiny_obj_loader.h>
#include <entity/bounding_box.h>

namespace naiive::entity {
class ModelDefault {
 public:
  ModelDefault(ID3D11Device* device, const std::string raw_path);
  ~ModelDefault() { Shutdown(); }

  const std::string& Name() const { return raw_path_; }
  DirectX::XMFLOAT4 Vertex(UINT tri_id, UINT v_id) const {
    auto vi = shapes_[0].mesh.indices[3 * tri_id + 2 - v_id].vertex_index;
    auto x = attribute_.vertices[3 * vi];
    auto y = attribute_.vertices[3 * vi + 1];
    auto z = -attribute_.vertices[3 * vi + 2];
    return {x, y, z, 1};
  }
  DirectX::XMFLOAT2 TexCoord(UINT tri_id, UINT v_id) const {
    auto ti = shapes_[0].mesh.indices[3 * tri_id + 2 - v_id].texcoord_index;
    auto x = attribute_.texcoords[2 * ti];
    auto y = 1.0f - attribute_.texcoords[2 * ti + 1];
    return {x, y};
  }
  DirectX::XMFLOAT4 Normal(UINT tri_id, UINT v_id) const {
    auto ni = shapes_[0].mesh.indices[3 * tri_id + 2 - v_id].normal_index;
    auto x = attribute_.normals[3 * ni];
    auto y = attribute_.normals[3 * ni + 1];
    auto z = -attribute_.normals[3 * ni + 2];
    DirectX::XMFLOAT4 result = {x, y, z, 0};
    DirectX::XMVECTOR xmresult = DirectX::XMLoadFloat4(&result);
    xmresult = DirectX::XMVector3Normalize(xmresult);
    DirectX::XMStoreFloat4(&result, xmresult);
    return result;
  }
  UINT NumIndices() const { return num_indices_; }
  const tinyobj::material_t& Material() const { return materials_[0]; }
  const BoundingBox3D& Aabb() const { return aabb_; }
  DirectX::XMMATRIX Transform() const {
    return DirectX::XMMatrixMultiply(
        DirectX::XMMatrixRotationRollPitchYaw(rotation_pyr_.x, rotation_pyr_.y,
                                              rotation_pyr_.z),
        DirectX::XMMatrixTranslation(translate_.x, translate_.y, translate_.z));
  }
  ID3D11Buffer* VertexBuffer() const { return vertex_buffer_; }
  ID3D11Buffer* IndexBuffer() const { return index_buffer_; }
  ID3D11ShaderResourceView* DiffuseMap() const { return srv_diffuse_map_; }
  ID3D11ShaderResourceView* BumpMap() const { return srv_bump_map_; }

  void MoveTo(const DirectX::XMFLOAT3& translate) { translate_ = translate; }

 private:
  void Initialize(ID3D11Device* device);
  void Shutdown();

  // Transform
  std::string raw_path_;
  DirectX::XMFLOAT3 translate_;
  DirectX::XMFLOAT3 rotation_pyr_;

  // Mesh
  tinyobj::attrib_t attribute_;
  std::vector<tinyobj::shape_t> shapes_;
  std::vector<tinyobj::material_t> materials_;

  // Resources
  UINT num_indices_ = 0;
  BoundingBox3D aabb_;
  ID3D11Buffer* vertex_buffer_ = nullptr;
  ID3D11Buffer* index_buffer_ = nullptr;
  ID3D11ShaderResourceView* srv_diffuse_map_ = nullptr;
  ID3D11ShaderResourceView* srv_bump_map_ = nullptr;
};
}  // namespace naiive::entity
#endif
