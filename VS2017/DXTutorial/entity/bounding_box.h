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

#ifndef __ENTITY_BOUNDING_BOX__
#define __ENTITY_BOUNDING_BOX__
#include <algorithm>
#include <limits>

#include <DirectXMath.h>

#include <utils/debug.h>

namespace naiive::entity {
class BoundingBox3D {
 public:
  BoundingBox3D() {
    FLOAT low = std::numeric_limits<FLOAT>::lowest();
    FLOAT high = std::numeric_limits<FLOAT>::max();
    min_ = DirectX::XMFLOAT3(high, high, high);
    max_ = DirectX::XMFLOAT3(low, low, low);
  }
  const DirectX::XMFLOAT3& operator[](int i) const {
    ASSERT(i == 0 || i == 1);
    if (i == 0) {
      return min_;
    } else {
      return max_;
    }
  }
  DirectX::XMFLOAT3& operator[](int i) {
    ASSERT(i == 0 || i == 1);
    if (i == 0) {
      return min_;
    } else {
      return max_;
    }
  }

  DirectX::XMFLOAT3 Corner(int corner) const {
    return DirectX::XMFLOAT3((*this)[(corner & 1)].x,
                             (*this)[(corner & 2) >> 1].y,
                             (*this)[(corner & 4) >> 2].z);
  }

  DirectX::XMFLOAT3 Diagonal() const {
    DirectX::XMFLOAT3 ret;
    DirectX::XMVECTOR xmin = DirectX::XMLoadFloat3(&min_);
    DirectX::XMVECTOR xmax = DirectX::XMLoadFloat3(&max_);
    DirectX::XMStoreFloat3(&ret, DirectX::XMVectorSubtract(xmax, xmin));
    return ret;
  }

  FLOAT SurfaceArea() const {
    DirectX::XMFLOAT3 diag = Diagonal();
    return 2 * (diag.x * diag.y + diag.x * diag.z + diag.y * diag.z);
  }

  FLOAT Volume() const {
    DirectX::XMFLOAT3 diag = Diagonal();
    return diag.x * diag.y * diag.z;
  }

  void Add(const DirectX::XMFLOAT3& rhs) {
    min_.x = std::min(min_.x, rhs.x);
    min_.y = std::min(min_.y, rhs.y);
    min_.z = std::min(min_.z, rhs.z);
    max_.x = std::max(max_.x, rhs.x);
    max_.y = std::max(max_.y, rhs.y);
    max_.z = std::max(max_.z, rhs.z);
  }

  BoundingBox3D operator+(const BoundingBox3D& rhs) const {
    BoundingBox3D ret(*this);
    ret.Add(rhs.min_);
    ret.Add(rhs.max_);
    return ret;
  }
  BoundingBox3D& operator+=(const BoundingBox3D& rhs) {
    Add(rhs.min_);
    Add(rhs.max_);
    return *this;
  }
  BoundingBox3D operator*(const BoundingBox3D& rhs) const {
    BoundingBox3D ret(*this);
    ret.min_.x = std::max(ret.min_.x, rhs.min_.x);
    ret.max_.x = std::min(ret.max_.x, rhs.max_.x);

    ret.min_.y = std::max(ret.min_.y, rhs.min_.y);
    ret.max_.y = std::min(ret.max_.y, rhs.max_.y);

    ret.min_.z = std::max(ret.min_.z, rhs.min_.z);
    ret.max_.z = std::min(ret.max_.z, rhs.max_.z);
    return ret;
  }
  BoundingBox3D& operator*=(const BoundingBox3D& rhs) {
    min_.x = std::max(min_.x, rhs.min_.x);
    max_.x = std::min(max_.x, rhs.max_.x);

    min_.y = std::max(min_.y, rhs.min_.y);
    max_.y = std::min(max_.y, rhs.max_.y);

    min_.z = std::max(min_.z, rhs.min_.z);
    max_.z = std::min(max_.z, rhs.max_.z);
    return *this;
  }

 private:
  DirectX::XMFLOAT3 min_, max_;
};
}  // namespace naiive::entity
#endif
