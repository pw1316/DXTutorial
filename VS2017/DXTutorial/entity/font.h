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

#ifndef __ENTITY_FONT__
#define __ENTITY_FONT__
#include <string>

namespace naiive::entity {
class Font {
  struct FontType {
    FLOAT uv_left, uv_right;
    UINT size;
  };
  struct CB0Type {
    DirectX::XMFLOAT4X4 matrix_proj;
    DirectX::XMFLOAT4 font_color;
  };
  static constexpr UINT kStartChar = 32U;
  static constexpr UINT kNumChar = 95U;

 public:
  Font() = default;
  ~Font() = default;

  void Initialize(ID3D11Device* device);
  void Shutdown();
  void Render(ID3D11Device* device, ID3D11DeviceContext* context,
              const std::string& text, const DirectX::XMFLOAT2& pos,
              DirectX::XMFLOAT4X4 proj);

 private:
  /* Resources */
  void InitializeBuffer(ID3D11Device* device);
  void ShutdownBuffer();

  /* Shader */
  void InitializeShader(ID3D11Device* device);
  void ShutdownShader();

  FontType font_[kNumChar];

  /* Resources */
  ID3D11Buffer* vertex_buffer_ = nullptr;
  ID3D11Buffer* index_buffer_ = nullptr;
  ID3D11Buffer* const_buffer_0_ = nullptr;
  ID3D11ShaderResourceView* shader_resource_font_texture_ = nullptr;
  ID3D11SamplerState* sampler_state_ = nullptr;

  /* Shader */
  ID3D11VertexShader* vertex_shader_ = nullptr;
  ID3D11PixelShader* pixel_shader_ = nullptr;
  ID3D11InputLayout* input_layout_ = nullptr;
};
}  // namespace naiive::entity

#endif
