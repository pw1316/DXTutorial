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

#ifndef __ENTITY_RENDER_TO_TEXTURE__
#define __ENTITY_RENDER_TO_TEXTURE__

namespace naiive::entity {
class RenderToTexture {
  struct VBType {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
  };
  struct CB0Type {
    DirectX::XMFLOAT4X4 matrix_proj;
  };
  static constexpr ID3D11ShaderResourceView* kDummyShaderResourceView = nullptr;
  static constexpr UINT kWidth = 256;
  static constexpr UINT kHeight = 192;

 public:
  void Initialize(ID3D11Device* device, UINT width, UINT height);
  void Shutdown();
  void Render(ID3D11DeviceContext* context, const DirectX::XMFLOAT2& pos,
              DirectX::XMFLOAT4X4 proj);

  ID3D11RenderTargetView* render_target_view() { return render_target_view_; }
  ID3D11ShaderResourceView* shader_resource_view() {
    return shader_resource_view_;
  }

 private:
  void InitializeShader(ID3D11Device* device);
  void ShutdownShader();

  ID3D11RenderTargetView* render_target_view_ = nullptr;
  ID3D11ShaderResourceView* shader_resource_view_ = nullptr;
  ID3D11Texture2D* back_buffer_ = nullptr;

  ID3D11Buffer* const_buffer_ = nullptr;
  ID3D11Buffer* vertex_buffer_ = nullptr;
  ID3D11Buffer* index_buffer_ = nullptr;
  ID3D11SamplerState* sampler_state_ = nullptr;

  ID3D11VertexShader* vertex_shader_ = nullptr;
  ID3D11PixelShader* pixel_shader_ = nullptr;
  ID3D11InputLayout* input_layout_ = nullptr;
};
}  // namespace naiive::entity
#endif
