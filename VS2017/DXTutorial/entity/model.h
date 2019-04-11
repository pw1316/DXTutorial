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

#ifndef __ENTITY_MODEL__
#define __ENTITY_MODEL__
#include <string>
#include <vector>

#include <3rdparty/include/tiny_obj_loader.h>
#include <entity/bounding_box.h>

namespace naiive::entity {
class Model3D {
  struct TinyObj {
    tinyobj::attrib_t attr;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
  };

 public:
  explicit Model3D(const std::string path)
      : name_(path),
        translate_(0.0f, 0.0f, 0.0f),
        rotation_pyr_(0.0f, 0.0f, 0.0f) {}

  void Initialize(ID3D11Device* device);
  void Shutdown();

  DirectX::XMMATRIX Transform() const {
    return DirectX::XMMatrixMultiply(
        DirectX::XMMatrixRotationRollPitchYaw(rotation_pyr_.x, rotation_pyr_.y,
                                              rotation_pyr_.z),
        DirectX::XMMatrixTranslation(translate_.x, translate_.y, translate_.z));
  }
  DirectX::XMFLOAT3 Corner(int i) const { return aabb_.Corner(i); }
  ID3D11Buffer* VertexBuffer() const { return vertex_buffer_; }
  ID3D11Buffer* IndexBuffer() const { return index_buffer_; }
  ID3D11ShaderResourceView* const* ShaderResource() const {
    return &shader_resource_texture_[0];
  }
  UINT vertex_number() const { return vertex_number_; }
  const tinyobj::material_t& Material() const { return obj.materials[0]; }
  void MoveTo(const DirectX::XMFLOAT3& translate) { translate_ = translate; }

 private:
  /* Resources */
  void InitializeBuffer(ID3D11Device* device);
  void ShutdownBuffer();

  std::string name_;
  /* Transform */
  DirectX::XMFLOAT3 translate_;
  DirectX::XMFLOAT3 rotation_pyr_;

  /* Resources */
  TinyObj obj;
  UINT vertex_number_ = 0;
  ID3D11Buffer* vertex_buffer_ = nullptr;
  ID3D11Buffer* index_buffer_ = nullptr;
  BoundingBox3D aabb_;
  ID3D11ShaderResourceView* shader_resource_texture_[3] = {nullptr, nullptr,
                                                           nullptr};
};
}  // namespace naiive::entity
#endif
