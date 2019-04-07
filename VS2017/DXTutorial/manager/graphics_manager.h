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

#ifndef __MANAGER_GRAPHICS_MANAGER__
#define __MANAGER_GRAPHICS_MANAGER__
#include <random>
#include <vector>

#include <DirectXMath.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>

#include <core/iview.h>

namespace naiive::entity {
class Model3D;
class Font;
class RenderToTexture;
}  // namespace naiive::entity

namespace naiive::manager {
class GraphicsManagerClass : public core::IView {
  friend GraphicsManagerClass& GraphicsManager();
  class Camera {
   public:
    void SetPos(FLOAT xx, FLOAT yy, FLOAT zz) {
      x_ = xx;
      y_ = yy;
      z_ = zz;
      dirty_ = true;
    }
    void SetRot(FLOAT xx, FLOAT yy, FLOAT zz) {
      rot_x_ = xx;
      rot_y_ = yy;
      rot_z_ = zz;
      dirty_ = true;
    }
    DirectX::XMFLOAT4 GetPos() const {
      return DirectX::XMFLOAT4(x_, y_, z_, 1.0f);
    }
    DirectX::XMFLOAT3 GetRot() const {
      return DirectX::XMFLOAT3(rot_x_, rot_y_, rot_z_);
    }
    void GetMatrix(DirectX::XMFLOAT4X4& m) {
      if (dirty_) {
        DirectX::XMFLOAT3 pos_raw(x_, y_, z_);
        DirectX::XMFLOAT3 up_raw(0.0f, 1.0f, 0.0f);
        DirectX::XMFLOAT3 look_at_raw(0.0f, 0.0f, 1.0f);
        auto rot_matrix =
            DirectX::XMMatrixRotationRollPitchYaw(rot_x_, rot_y_, rot_z_);
        auto pos = DirectX::XMLoadFloat3(&pos_raw);
        auto look_at = DirectX::XMVector3TransformCoord(
            DirectX::XMLoadFloat3(&look_at_raw), rot_matrix);
        auto up = DirectX::XMVector3TransformCoord(
            DirectX::XMLoadFloat3(&up_raw), rot_matrix);
        look_at = DirectX::XMVectorAdd(pos, look_at);
        DirectX::XMStoreFloat4x4(&matrix_view_,
                                 DirectX::XMMatrixLookAtLH(pos, look_at, up));
        dirty_ = false;
      }
      m = matrix_view_;
    }

   private:
    BOOL dirty_ = true;
    FLOAT x_ = 0.0f, y_ = 0.0f, z_ = 0.0f;
    FLOAT rot_x_ = 0.0f, rot_y_ = 0.0f, rot_z_ = 0.0f;
    DirectX::XMFLOAT4X4 matrix_view_{};
  };
  struct Light {
    DirectX::XMFLOAT3 dir{};
  };

 public:
  /* IView */
  virtual void Initialize(HWND hwnd, UINT width, UINT height) override;
  virtual void Shutdown() override;
  virtual BOOL OnUpdate() override;

 private:
  GraphicsManagerClass()
      : rng_(0), distribution_xy_(-40, 40), distribution_z_(-5, 40) {}

  /* D3D Basic */
  void InitializeDevice(HWND hwnd, UINT width, UINT height);
  void ShutdownDevice();
  void DebugDevice();

  /* Output Merger */
  void InitializeOM(HWND hwnd, UINT width, UINT height);
  void ShutdownOM();

  /* Output Merger */
  void InitializeRasterizer(HWND hwnd, UINT width, UINT height);
  void ShutdownRasterizer();

  void BeginScene(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv) {
    float color[] = {0.0f, 0.0f, 0.0f, 0.0f};
    device_context_->ClearRenderTargetView(rtv, color);
    device_context_->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
  }
  void EndScene() { swap_chain_->Present(1, 0); }

  /* D3D Basic */
  ID3D11Device* device_ = nullptr;
  ID3D11DeviceContext* device_context_ = nullptr;
  ID3D11Debug* debug_ = nullptr;
  IDXGISwapChain* swap_chain_ = nullptr;

  /* Output Merger */
  ID3D11RenderTargetView* render_target_view_ = nullptr;
  ID3D11DepthStencilView* depth_stencil_view_ = nullptr;
  ID3D11DepthStencilState* depth_stencil_state_z_on_ = nullptr;
  ID3D11DepthStencilState* depth_stencil_state_z_off_ = nullptr;
  ID3D11BlendState* blend_state_alpha_on_ = nullptr;
  ID3D11BlendState* blend_state_alpha_off_ = nullptr;

  /* Rasterizer */
  ID3D11RasterizerState* rasterizer_state_ = nullptr;

  DirectX::XMFLOAT4X4 matrix_perspective_{};
  DirectX::XMFLOAT4X4 matrix_orthogonal_{};

  /* RNG */
  std::mt19937 rng_;
  std::uniform_real_distribution<FLOAT> distribution_xy_;
  std::uniform_real_distribution<FLOAT> distribution_z_;

  Camera camera_;
  Light light_;
  naiive::entity::Model3D* model_ = nullptr;
  std::vector<DirectX::XMFLOAT3> model_dup_;
  naiive::entity::Font* gui_ = nullptr;
  std::shared_ptr<entity::RenderToTexture> render_to_texture_;
};

GraphicsManagerClass& GraphicsManager();
}  // namespace naiive::manager

#endif
