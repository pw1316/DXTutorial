#ifndef __MANAGER_GRAPHICS_MANAGER__
#define __MANAGER_GRAPHICS_MANAGER__
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#include <stdafx.h>

#include <D3D11.h>
#include <D3DX10math.h>
#include <DXGI.h>

#include <Core/IGraphics.hpp>
#include <Core/Interface/IView.hpp>
#include <Entity/Font.hpp>
#include <Entity/Model3D.hpp>

namespace Naiive::Manager {
class GraphicsManagerClass : public Core::IView, public Core::IGraphics {
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
    D3DXVECTOR4 GetPos() const { return D3DXVECTOR4(m_x, m_y, m_z, 1.0f); }
    D3DXVECTOR3 GetRot() const { return D3DXVECTOR3(m_rotx, m_roty, m_rotz); }
    void GetMatrix(D3DXMATRIX& m) {
      if (m_dirty) {
        D3DXVECTOR3 pos(m_x, m_y, m_z), up(0.0f, 1.0f, 0.0f),
            lookAt(0.0f, 0.0f, 1.0f);
        D3DXMATRIX rotMatrix{};
        D3DXMatrixRotationYawPitchRoll(&rotMatrix, m_roty, m_rotx, m_rotz);
        D3DXVec3TransformCoord(&lookAt, &lookAt, &rotMatrix);
        D3DXVec3TransformCoord(&up, &up, &rotMatrix);
        lookAt = pos + lookAt;
        D3DXMatrixLookAtLH(&m_matrix, &pos, &lookAt, &up);
        m_dirty = false;
      }
      m = m_matrix;
    }

   private:
    BOOL m_dirty = true;
    FLOAT m_x = 0.0f, m_y = 0.0f, m_z = 0.0f;
    FLOAT m_rotx = 0.0f, m_roty = 0.0f, m_rotz = 0.0f;
    D3DXMATRIX m_matrix{};
  };
  struct Light {
    D3DXVECTOR3 m_dir{};
  };

 public:
  /* IView */
  virtual void Awake() override {}
  virtual void Destroy() override {}
  virtual void OnMessage(const Naiive::Core::Message& msg) override {}

  /* IGraphics */
  virtual void Initialize(HWND hWnd, UINT width, UINT height) override;
  virtual void Shutdown() override;
  virtual BOOL OnUpdate() override;
  virtual LRESULT CALLBACK MessageHandler(HWND hWnd, UINT message,
                                          WPARAM wParam,
                                          LPARAM lParam) override;

 private:
  GraphicsManagerClass() = default;

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
  void EndScene() { m_swapChain->Present(1, 0); }

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

  D3DXMATRIX m_MatrixProj{};
  D3DXMATRIX m_MatrixOrtho{};

  Camera m_camera;
  PW::Entity::Model3D* m_model = nullptr;
  PW::Entity::Font* m_gui = nullptr;
  Light m_light;
};

GraphicsManagerClass& GraphicsManager();
}  // namespace Naiive::Manager

#endif
