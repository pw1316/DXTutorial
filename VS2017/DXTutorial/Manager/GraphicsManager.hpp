#ifndef __PW_MANAGER_GRAPHICS_MANAGER__
#define __PW_MANAGER_GRAPHICS_MANAGER__
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#include "stdafx.h"

#include <DXGI.h>
#include <D3D11.h>
#include <D3DX10math.h>

#include <Core/Interface/IView.hpp>
#include <Entity/Model3D.hpp>
#include <Entity/Font.hpp>
#include "Camera.h"
#include "Light.h"

namespace PW
{
    namespace Manager
    {
        class GraphicsManager : public Core::IView
        {
        public:
            GraphicsManager() = default;
            virtual ~GraphicsManager() = default;

            /* Override */
            virtual HRESULT Awake() override { return S_OK; }
            virtual void Destroy() override {}
            virtual void OnMessage(const Core::Message &msg) override {}

            void Initialize(HWND hwnd, UINT w, UINT h);
            void Shutdown();
            void OnRender(float f);
        private:
            void GetRefreshRate(UINT w, UINT h, UINT &num, UINT &den);

            /* D3D Basic */
            void InitializeDevice(HWND hwnd, UINT w, UINT h);
            void ShutdownDevice();

            /* Output Merger */
            void InitializeOM(HWND hwnd, UINT w, UINT h);
            void ShutdownOM();

            /* Output Merger */
            void InitializeRasterizer(HWND hwnd, UINT w, UINT h);
            void ShutdownRasterizer();

            void BeginScene()
            {
                float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
                m_deviceContext->ClearRenderTargetView(m_RTView, color);
                m_deviceContext->ClearDepthStencilView(m_DSView, D3D11_CLEAR_DEPTH, 1.0f, 0);
            }
            void EndScene()
            {
                m_swapChain->Present(1, 0);
            }

            /* D3D Basic */
            IDXGISwapChain *m_swapChain = nullptr;
            ID3D11Device *m_device = nullptr;
            ID3D11DeviceContext *m_deviceContext = nullptr;

            /* Output Merger */
            ID3D11RenderTargetView *m_RTView = nullptr;
            ID3D11DepthStencilView *m_DSView = nullptr;
            ID3D11DepthStencilState *m_DSStateWithZ = nullptr;
            ID3D11DepthStencilState *m_DSStateWithoutZ = nullptr;

            /* Rasterizer */
            ID3D11RasterizerState *m_RState = nullptr;

            D3DXMATRIX m_MatrixProj{};
            D3DXMATRIX m_MatrixWorld{};// TODO move this to model
            D3DXMATRIX m_MatrixOrtho{};

            Camera *m_camera = nullptr;
            Entity::Model3D *m_model = nullptr;
            Entity::Font *m_gui = nullptr;
            Light *m_light = nullptr;
        };
    }
}

#endif
