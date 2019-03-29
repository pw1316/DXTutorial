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

#include <DirectXMath.h>
#include <d3d11_4.h>

#include <3rdparty/include/tiny_obj_loader.h>
#include <entity/bounding_box.h>

namespace naiive::entity {
class Model3D {
 private:
  struct VBType {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT3 tangent;
    DirectX::XMFLOAT3 binormal;
  };
  struct CBTransformType {
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 proj;
  };
  struct CBCameraLightType {
    DirectX::XMFLOAT4 camera_pos;
    DirectX::XMFLOAT3 light_dir;
    float padding;
  };
  struct CBMaterialType {
    DirectX::XMFLOAT4 ka;
    DirectX::XMFLOAT4 kd;
    DirectX::XMFLOAT4 ks;
    float ns;
    DirectX::XMFLOAT3 padding;
  };
  struct TinyObj {
    tinyobj::attrib_t attr;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
  };

 public:
  Model3D() = delete;
  explicit Model3D(const std::string path)
      : name_(path),
        translate_(0.0f, 0.0f, 0.0f),
        rotation_pyr_(0.0f, 0.0f, 0.0f) {}
  ~Model3D() = default;

  void Initialize(ID3D11Device* device);
  void Shutdown();
  BOOL Render(ID3D11DeviceContext* context, const DirectX::XMFLOAT4X4& view,
              const DirectX::XMFLOAT4X4& proj,
              const DirectX::XMFLOAT4& camera_pos,
              const DirectX::XMFLOAT3& dir);
  void MoveTo(const DirectX::XMFLOAT3& translate) { translate_ = translate; }

 private:
  /* Resources */
  void InitializeBuffer(ID3D11Device* device);
  void ShutdownBuffer();

  /* Shader */
  void InitializeShader(ID3D11Device* device);
  void ShutdownShader();

  std::string name_;
  /* Transform */
  DirectX::XMFLOAT3 translate_;
  DirectX::XMFLOAT3 rotation_pyr_;

  /* Resources */
  UINT vertex_number_ = 0;
  ID3D11Buffer* vertex_buffer_ = nullptr;
  ID3D11Buffer* index_buffer_ = nullptr;
  ID3D11Buffer* const_buffer_transform_ = nullptr;
  ID3D11Buffer* const_buffer_camera_light_ = nullptr;
  ID3D11Buffer* const_buffer_material_ = nullptr;
  ID3D11ShaderResourceView* shader_resource_texture_[3] = {nullptr, nullptr,
                                                           nullptr};
  ID3D11SamplerState* sampler_state_ = nullptr;
  BoundingBox3D aabb_;

  /* Shader */
  ID3D11VertexShader* vertex_shader_ = nullptr;
  ID3D11PixelShader* pixel_shader_ = nullptr;
  ID3D11InputLayout* input_layout_ = nullptr;
};
}  // namespace naiive::entity
#endif
