#ifndef __MANAGER_GRAPHICS_MANAGER__
#define __MANAGER_GRAPHICS_MANAGER__
#include <random>
#include <vector>

#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi.h>

#include <core/iview.h>

namespace naiive::entity {
class Model3D;
class Font;
}  // namespace naiive::entity

namespace naiive::Manager {
class GraphicsManagerClass : public core::IView {
  friend GraphicsManagerClass& GraphicsManager();
  struct Camera {
   public:
    void SetPos(FLOAT xx, FLOAT yy, FLOAT zz) {
      m_x = xx;
      m_y = yy;
      m_z = zz;
      m_dirty = true;
    }
    void SetRot(FLOAT xx, FLOAT yy, FLOAT zz) {
      m_rotx = xx;
      m_roty = yy;
      m_rotz = zz;
      m_dirty = true;
    }
    DirectX::XMFLOAT4 GetPos() const {
      return DirectX::XMFLOAT4(m_x, m_y, m_z, 1.0f);
    }
    DirectX::XMFLOAT3 GetRot() const {
      return DirectX::XMFLOAT3(m_rotx, m_roty, m_rotz);
    }
    void GetMatrix(DirectX::XMFLOAT4X4& m) {
      if (m_dirty) {
        DirectX::XMFLOAT3 pos_raw(m_x, m_y, m_z);
        DirectX::XMFLOAT3 up_raw(0.0f, 1.0f, 0.0f);
        DirectX::XMFLOAT3 lookAt_raw(0.0f, 0.0f, 1.0f);
        auto rotMatrix =
            DirectX::XMMatrixRotationRollPitchYaw(m_rotx, m_roty, m_rotz);
        auto pos = DirectX::XMLoadFloat3(&pos_raw);
        auto lookAt = DirectX::XMVector3TransformCoord(
            DirectX::XMLoadFloat3(&lookAt_raw), rotMatrix);
        auto up = DirectX::XMVector3TransformCoord(
            DirectX::XMLoadFloat3(&up_raw), rotMatrix);
        lookAt = DirectX::XMVectorAdd(pos, lookAt);
        DirectX::XMStoreFloat4x4(&m_matrix,
                                 DirectX::XMMatrixLookAtLH(pos, lookAt, up));
        m_dirty = false;
      }
      m = m_matrix;
    }

   private:
    BOOL m_dirty = true;
    FLOAT m_x = 0.0f, m_y = 0.0f, m_z = 0.0f;
    FLOAT m_rotx = 0.0f, m_roty = 0.0f, m_rotz = 0.0f;
    DirectX::XMFLOAT4X4 m_matrix{};
  };
  struct Light {
    DirectX::XMFLOAT3 m_dir{};
  };

 public:
  /* IView */
  virtual void Initialize(HWND hWnd, UINT width, UINT height) override;
  virtual void Shutdown() override;
  virtual BOOL OnUpdate() override;

 private:
  GraphicsManagerClass() : m_rng(m_dev()), m_distXY(-40, 40), m_distZ(-5, 40) {}

  void GetRefreshRate(UINT w, UINT h, UINT& num, UINT& den);

  /* D3D Basic */
  void InitializeDevice(HWND hwnd, UINT w, UINT h);
  void ShutdownDevice();

  /* Output Merger */
  void InitializeOM(HWND hwnd, UINT w, UINT h);
  void ShutdownOM();

  /* Output Merger */
  void InitializeRasterizer(HWND hwnd, UINT w, UINT h);
  void ShutdownRasterizer();

  void BeginScene() {
    float color[] = {0.0f, 0.0f, 0.0f, 0.0f};
    m_deviceContext->ClearRenderTargetView(m_RTView, color);
    m_deviceContext->ClearDepthStencilView(m_DSView, D3D11_CLEAR_DEPTH, 1.0f,
                                           0);
  }
  void EndScene() { m_swapChain->Present(0, 0); }

  /* D3D Basic */
  IDXGISwapChain* m_swapChain = nullptr;
  ID3D11Device* m_device = nullptr;
  ID3D11DeviceContext* m_deviceContext = nullptr;

  /* Output Merger */
  ID3D11RenderTargetView* m_RTView = nullptr;
  ID3D11DepthStencilView* m_DSView = nullptr;
  ID3D11DepthStencilState* m_DSStateWithZ = nullptr;
  ID3D11DepthStencilState* m_DSStateWithoutZ = nullptr;
  ID3D11BlendState* m_BlendStateWithAlpha = nullptr;
  ID3D11BlendState* m_BlendStateWithoutAlpha = nullptr;

  /* Rasterizer */
  ID3D11RasterizerState* m_RState = nullptr;

  DirectX::XMFLOAT4X4 m_MatrixProj{};
  DirectX::XMFLOAT4X4 m_MatrixOrtho{};

  /* RNG */
  std::random_device m_dev;
  std::mt19937 m_rng;
  std::uniform_real_distribution<FLOAT> m_distXY;
  std::uniform_real_distribution<FLOAT> m_distZ;

  Camera m_camera;
  Light m_light;
  naiive::entity::Model3D* m_model = nullptr;
  std::vector<DirectX::XMFLOAT3> m_model_dup;
  naiive::entity::Font* m_gui = nullptr;
};

GraphicsManagerClass& GraphicsManager();
}  // namespace naiive::Manager

#endif
