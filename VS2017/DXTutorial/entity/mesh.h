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
class Mesh {
 public:
  explicit Mesh(const std::string& raw_path);

 private:
  std::string path_;
  tinyobj::attrib_t attribute_;
  std::vector<tinyobj::shape_t> shapes_;
  std::vector<tinyobj::material_t> materials_;
};
}  // namespace naiive::entity
#endif
