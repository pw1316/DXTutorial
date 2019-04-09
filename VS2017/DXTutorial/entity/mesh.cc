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

#include "mesh.h"

#include <filesystem>

namespace naiive::entity {
Mesh::Mesh(const std::string& raw_path) : path_(raw_path) {
  std::filesystem::path path(".");
  path /= raw_path;
  path += ".obj";
  path.make_preferred();
  std::string obj_warning;
  std::string obj_error;
  auto res = tinyobj::LoadObj(&attribute_, &shapes_, &materials_, &obj_warning,
                              &obj_error, path.string().c_str(),
                              path.parent_path().string().c_str(), TRUE);
  if (!obj_warning.empty()) {
    LOG(LOG_WARN)(obj_warning);
  }
  if (!obj_error.empty()) {
    LOG(LOG_ERROR)(obj_error);
  }
}
}  // namespace naiive::entity
