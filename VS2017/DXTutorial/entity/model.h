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

#include <entity/bounding_box.h>
#include <entity/mesh.h>
#include <entity/shader/shader.h>

namespace naiive::entity {
class Model3D {
 public:
  explicit Model3D(const std::string& mesh_path, const std::string& shader_path)
      : mesh_(new Mesh(mesh_path)),
        shader_(new Shader(shader_path)),
        translate_(0.0f, 0.0f, 0.0f),
        rotation_pyr_(0.0f, 0.0f, 0.0f) {}
  ~Model3D() = default;

  void Initialize(ID3D11Device* device);
  void Shutdown();
  BOOL Render(ID3D11DeviceContext* context, const DirectX::XMFLOAT4X4& view,
              const DirectX::XMFLOAT4X4& proj,
              const DirectX::XMFLOAT4& camera_pos,
              const DirectX::XMFLOAT4& dir);
  void MoveTo(const DirectX::XMFLOAT3& translate) { translate_ = translate; }

 private:
  std::shared_ptr<Mesh> mesh_;
  std::shared_ptr<Shader> shader_;
  BoundingBox3D aabb_;

  /* Transform */
  DirectX::XMFLOAT3 translate_;
  DirectX::XMFLOAT3 rotation_pyr_;

  /* Resources */
  UINT stride_ = 0U;
  ID3D11Buffer* vertex_buffer_ = nullptr;
  ID3D11Buffer* index_buffer_ = nullptr;
};
}  // namespace naiive::entity
#endif
