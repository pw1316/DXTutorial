#ifndef __ENTITY_BOUNDING_BOX__
#define __ENTITY_BOUNDING_BOX__
#include <stdafx.h>

#include <algorithm>
#include <limits>

#include <DirectXMath.h>

namespace Naiive::Entity {
class BoundingBox3D {
 public:
  BoundingBox3D() {
    FLOAT low = std::numeric_limits<FLOAT>::lowest();
    FLOAT high = std::numeric_limits<FLOAT>::max();
    pMin = DirectX::XMFLOAT3(high, high, high);
    pMax = DirectX::XMFLOAT3(low, low, low);
  }
  const DirectX::XMFLOAT3& operator[](int i) const {
    if (i == 0) {
      return pMin;
    } else if (i == 1) {
      return pMax;
    }
    assert(FALSE);
  }
  DirectX::XMFLOAT3& operator[](int i) {
    if (i == 0) {
      return pMin;
    } else if (i == 1) {
      return pMax;
    }
    assert(FALSE);
  }

  DirectX::XMFLOAT3 Corner(int corner) const {
    return DirectX::XMFLOAT3((*this)[(corner & 1)].x,
                             (*this)[(corner & 2) >> 1].y,
                             (*this)[(corner & 4) >> 2].z);
  }

  DirectX::XMFLOAT3 Diagonal() const {
    DirectX::XMFLOAT3 ret;
    DirectX::XMVECTOR xmin = DirectX::XMLoadFloat3(&pMin);
    DirectX::XMVECTOR xmax = DirectX::XMLoadFloat3(&pMax);
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
    pMin.x = std::min(pMin.x, rhs.x);
    pMin.y = std::min(pMin.y, rhs.y);
    pMin.z = std::min(pMin.z, rhs.z);
    pMax.x = std::max(pMax.x, rhs.x);
    pMax.y = std::max(pMax.y, rhs.y);
    pMax.z = std::max(pMax.z, rhs.z);
  }

  BoundingBox3D operator+(const BoundingBox3D& rhs) const {
    BoundingBox3D ret(*this);
    ret.Add(rhs.pMin);
    ret.Add(rhs.pMax);
    return ret;
  }
  BoundingBox3D& operator+=(const BoundingBox3D& rhs) {
    Add(rhs.pMin);
    Add(rhs.pMax);
    return *this;
  }
  BoundingBox3D operator*(const BoundingBox3D& rhs) const {
    BoundingBox3D ret(*this);
    ret.pMin.x = std::max(ret.pMin.x, rhs.pMin.x);
    ret.pMax.x = std::min(ret.pMax.x, rhs.pMax.x);

    ret.pMin.y = std::max(ret.pMin.y, rhs.pMin.y);
    ret.pMax.y = std::min(ret.pMax.y, rhs.pMax.y);

    ret.pMin.z = std::max(ret.pMin.z, rhs.pMin.z);
    ret.pMax.z = std::min(ret.pMax.z, rhs.pMax.z);
    return ret;
  }
  BoundingBox3D& operator*=(const BoundingBox3D& rhs) {
    pMin.x = std::max(pMin.x, rhs.pMin.x);
    pMax.x = std::min(pMax.x, rhs.pMax.x);

    pMin.y = std::max(pMin.y, rhs.pMin.y);
    pMax.y = std::min(pMax.y, rhs.pMax.y);

    pMin.z = std::max(pMin.z, rhs.pMin.z);
    pMax.z = std::min(pMax.z, rhs.pMax.z);
    return *this;
  }

  DirectX::XMFLOAT3 pMin, pMax;
};
}  // namespace Naiive::Entity
#endif
