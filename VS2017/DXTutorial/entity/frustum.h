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

#ifndef __ENTITY_FRUSTUM__
#define __ENTITY_FRUSTUM__
#include <utils/range.h>

namespace naiive::entity {
class FrustumWorld {
 public:
  FrustumWorld(const DirectX::XMFLOAT4X4& matrix_view_proj) {
    /* near */
    DirectX::XMStoreFloat4(&plane_[0],
                           DirectX::XMPlaneNormalize(DirectX::XMVectorSet(
                               matrix_view_proj._13, matrix_view_proj._23,
                               matrix_view_proj._33, matrix_view_proj._43)));

    /* far */
    DirectX::XMStoreFloat4(&plane_[1],
                           DirectX::XMPlaneNormalize(DirectX::XMVectorSet(
                               matrix_view_proj._14 - matrix_view_proj._13,
                               matrix_view_proj._24 - matrix_view_proj._23,
                               matrix_view_proj._34 - matrix_view_proj._33,
                               matrix_view_proj._44 - matrix_view_proj._43)));

    /* left */
    DirectX::XMStoreFloat4(&plane_[2],
                           DirectX::XMPlaneNormalize(DirectX::XMVectorSet(
                               matrix_view_proj._14 + matrix_view_proj._11,
                               matrix_view_proj._24 + matrix_view_proj._21,
                               matrix_view_proj._34 + matrix_view_proj._31,
                               matrix_view_proj._44 + matrix_view_proj._41)));

    /* right */
    DirectX::XMStoreFloat4(&plane_[3],
                           DirectX::XMPlaneNormalize(DirectX::XMVectorSet(
                               matrix_view_proj._14 - matrix_view_proj._11,
                               matrix_view_proj._24 - matrix_view_proj._21,
                               matrix_view_proj._34 - matrix_view_proj._31,
                               matrix_view_proj._44 - matrix_view_proj._41)));

    /* bottom */
    DirectX::XMStoreFloat4(&plane_[4],
                           DirectX::XMPlaneNormalize(DirectX::XMVectorSet(
                               matrix_view_proj._14 + matrix_view_proj._12,
                               matrix_view_proj._24 + matrix_view_proj._22,
                               matrix_view_proj._34 + matrix_view_proj._32,
                               matrix_view_proj._44 + matrix_view_proj._42)));

    /* top */
    DirectX::XMStoreFloat4(&plane_[5],
                           DirectX::XMPlaneNormalize(DirectX::XMVectorSet(
                               matrix_view_proj._14 - matrix_view_proj._12,
                               matrix_view_proj._24 - matrix_view_proj._22,
                               matrix_view_proj._34 - matrix_view_proj._32,
                               matrix_view_proj._44 - matrix_view_proj._42)));
  }

  BOOL Check(const DirectX::XMFLOAT3& p) {
    for (auto&& plane : plane_) {
      auto xmres = DirectX::XMPlaneDotCoord(DirectX::XMLoadFloat4(&plane),
                                            DirectX::XMLoadFloat3(&p));
      FLOAT res;
      DirectX::XMStoreFloat(&res, xmres);
      if (res < 0.0f) {
        return FALSE;
      }
    }
    return TRUE;
  }

 private:
  DirectX::XMFLOAT4 plane_[6];
};
}  // namespace naiive::entity
#endif
