#ifndef __ENTITY_FRUSTUM__
#define __ENTITY_FRUSTUM__
#include <stdafx.h>

#include <DirectXMath.h>

#include <Utils/Range.hpp>

namespace Naiive::Entity {
class FrustumWorld {
 public:
  FrustumWorld(const DirectX::XMMATRIX& VP) {
    DirectX::XMFLOAT4X4 matrixVP;
    DirectX::XMStoreFloat4x4(&matrixVP, VP);
    /* near */
    DirectX::XMFLOAT4 temp(matrixVP._13, matrixVP._23, matrixVP._33,
                           matrixVP._43);
    m_plane[0] = DirectX::XMPlaneNormalize(DirectX::XMLoadFloat4(&temp));

    /* far */
    temp = DirectX::XMFLOAT4(
        matrixVP._14 - matrixVP._13, matrixVP._24 - matrixVP._23,
        matrixVP._34 - matrixVP._33, matrixVP._44 - matrixVP._43);
    m_plane[1] = DirectX::XMPlaneNormalize(DirectX::XMLoadFloat4(&temp));

    /* left */
    temp = DirectX::XMFLOAT4(
        matrixVP._14 + matrixVP._11, matrixVP._24 + matrixVP._21,
        matrixVP._34 + matrixVP._31, matrixVP._44 + matrixVP._41);
    m_plane[2] = DirectX::XMPlaneNormalize(DirectX::XMLoadFloat4(&temp));

    /* right */
    temp = DirectX::XMFLOAT4(
        matrixVP._14 - matrixVP._11, matrixVP._24 - matrixVP._21,
        matrixVP._34 - matrixVP._31, matrixVP._44 - matrixVP._41);
    m_plane[3] = DirectX::XMPlaneNormalize(DirectX::XMLoadFloat4(&temp));

    /* bottom */
    temp = DirectX::XMFLOAT4(
        matrixVP._14 + matrixVP._12, matrixVP._24 + matrixVP._22,
        matrixVP._34 + matrixVP._32, matrixVP._44 + matrixVP._42);
    m_plane[4] = DirectX::XMPlaneNormalize(DirectX::XMLoadFloat4(&temp));

    /* top */
    temp = DirectX::XMFLOAT4(
        matrixVP._14 - matrixVP._12, matrixVP._24 - matrixVP._22,
        matrixVP._34 - matrixVP._32, matrixVP._44 - matrixVP._42);
    m_plane[5] = DirectX::XMPlaneNormalize(DirectX::XMLoadFloat4(&temp));
  }

  BOOL Check(const DirectX::XMFLOAT3& p) {
    for (auto i : Utils::Range(6)) {
      auto xmres =
          DirectX::XMPlaneDotCoord(m_plane[i], DirectX::XMLoadFloat3(&p));
      FLOAT res;
      DirectX::XMStoreFloat(&res, xmres);
      if (res < 0.0f) {
        return FALSE;
      }
    }
    return TRUE;
  }

 private:
  DirectX::XMVECTOR m_plane[6];
};
}  // namespace Naiive::Entity
#endif
