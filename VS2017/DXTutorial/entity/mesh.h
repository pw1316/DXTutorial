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

#ifndef __ENTITY_MESH__
#define __ENTITY_MESH__
#include <string>

#include <3rdparty/include/tiny_obj_loader.h>

namespace naiive::entity {
// tinyobjloader wrapper
// The wrapper automatically convert OpenGL mesh to DirectX mesh. So:
// The mesh must have Right-handed coordinate system
// Texture coordinate must have OpenGL style
class Mesh {
 public:
  explicit Mesh(const std::string& raw_path);
  const std::string& get_path() const { return path_; }
  DirectX::XMFLOAT4 vertex(UINT triangle, UINT index) const {
    auto vi = shapes_[0].mesh.indices[3 * triangle + 2 - index].vertex_index;
    auto x = attribute_.vertices[3 * vi];
    auto y = attribute_.vertices[3 * vi + 1];
    auto z = -attribute_.vertices[3 * vi + 2];
    return {x, y, z, 1.0f};
  }
  DirectX::XMFLOAT2 texcoord(UINT triangle, UINT index) const {
    auto ti = shapes_[0].mesh.indices[3 * triangle + 2 - index].texcoord_index;
    auto x = attribute_.texcoords[2 * ti];
    auto y = attribute_.texcoords[2 * ti + 1];
    return {x, 1.0f - y};
  }
  DirectX::XMFLOAT4 normal(UINT triangle, UINT index) const {
    auto ni = shapes_[0].mesh.indices[3 * triangle + 2 - index].normal_index;
    auto x = attribute_.normals[3 * ni];
    auto y = attribute_.normals[3 * ni + 1];
    auto z = -attribute_.normals[3 * ni + 2];
    DirectX::XMFLOAT4 temp{x, y, z, 0.0f};
    DirectX::XMVECTOR xmtemp =
        DirectX::XMVector3Normalize(DirectX::XMLoadFloat4(&temp));
    DirectX::XMStoreFloat4(&temp, xmtemp);
    return temp;
  }
  UINT index_number() const {
    return static_cast<UINT>(shapes_[0].mesh.indices.size());
  }
  const tinyobj::material_t& material() const { return materials_[0]; }

 private:
  std::string path_;
  tinyobj::attrib_t attribute_;
  std::vector<tinyobj::shape_t> shapes_;
  std::vector<tinyobj::material_t> materials_;
};
}  // namespace naiive::entity
#endif
