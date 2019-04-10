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

#ifndef __ENTITY_SHADER__
#define __ENTITY_SHADER__
#include <string>

#include <entity/mesh.h>

namespace naiive::entity {
class Shader {
  struct VBType {
    DirectX::XMFLOAT4 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT4 normal;
    DirectX::XMFLOAT4 tangent;
    DirectX::XMFLOAT4 binormal;
  };

 public:
  explicit Shader(const std::string& raw_path) : path_(raw_path) {}
  void Initialize(ID3D11Device* device);
  void Shutdown();
  BOOL Render(ID3D11DeviceContext* context);
  HRESULT STDMETHODCALLTYPE CreateVertexBufferAndIndexBuffer(
      ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** pp_vertex_buffer,
      ID3D11Buffer** pp_index_buffer, UINT* p_stride);

 private:
  std::string path_;

  // Shader
  ID3D11VertexShader* vertex_shader_ = nullptr;
  ID3D11PixelShader* pixel_shader_ = nullptr;
  ID3D11InputLayout* input_layout_ = nullptr;

  // Shader Resource
};
}  // namespace naiive::entity
#endif
