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

#ifndef __ENTITY_SHADER_DEFAULT__
#define __ENTITY_SHADER_DEFAULT__
namespace naiive::entity {
class Model3D;

class ShaderDefault {
  struct CBTransformType {
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 proj;
  };
  struct CBCameraLightType {
    DirectX::XMFLOAT4 camera_pos;
    DirectX::XMFLOAT4 light_dir;
    DirectX::XMFLOAT4 fog;         // start, end, intensity, pad
    DirectX::XMFLOAT4 clip_plane;  // nx, ny, nz, -dot(n,p0)
  };
  struct CBMaterialType {
    DirectX::XMFLOAT4 ka;
    DirectX::XMFLOAT4 kd;
    DirectX::XMFLOAT4 ks;
    float ns;
    DirectX::XMFLOAT3 padding;
  };

 public:
  void Initialize(ID3D11Device* device);
  void Shutdown();
  BOOL Render(ID3D11DeviceContext* context, const Model3D& model,
              const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj,
              const DirectX::XMFLOAT4& camera_pos,
              const DirectX::XMFLOAT4& light_dir, const DirectX::XMFLOAT4& fog,
              const DirectX::XMFLOAT4& clip_plane);

 private:
  void InitializeShader(ID3D11Device* device);
  void ShutdownShader();
  void InitializeResource(ID3D11Device* device);
  void ShutdownResource();

  // Shader
  ID3D11VertexShader* vertex_shader_ = nullptr;
  ID3D11PixelShader* pixel_shader_ = nullptr;
  ID3D11InputLayout* input_layout_ = nullptr;

  // Resource
  ID3D11Buffer* const_buffer_transform_ = nullptr;
  ID3D11Buffer* const_buffer_camera_light_ = nullptr;
  ID3D11Buffer* const_buffer_material_ = nullptr;
  ID3D11SamplerState* sampler_state_ = nullptr;
};
}  // namespace naiive::entity
#endif
