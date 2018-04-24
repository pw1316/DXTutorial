#pragma once
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#include "stdafx.h"

#include <DXGI.h>
#include <D3D11.h>
#include <D3DX10math.h>

class Graphics
{
public:
    Graphics() {}
    Graphics(const Graphics& rhs) {}
    Graphics(Graphics&& rhs) {}
    ~Graphics() {}

    HRESULT Initialize(HWND hwnd, LONG w, LONG h);
    HRESULT Shutdown();
    HRESULT OnRender();

    ID3D11Device *GetDevice();
    ID3D11DeviceContext *GetDeviceContext();

    void GetProjectionMatrix(D3DXMATRIX&);
    void GetWorldMatrix(D3DXMATRIX&);
    void GetOrthoMatrix(D3DXMATRIX&);
private:
    HRESULT BeginScene(float r, float g, float b, float a);
    HRESULT EndScene();

    IDXGISwapChain *m_swapChain = nullptr;
    ID3D11Device *m_device = nullptr;
    ID3D11DeviceContext *m_deviceContext = nullptr;
    ID3D11RenderTargetView *m_renderTargetView = nullptr;
    ID3D11Texture2D *m_depthStencilBuffer = nullptr;
    ID3D11DepthStencilState *m_depthStencilState = nullptr;
    ID3D11DepthStencilView *m_depthStencilView = nullptr;
    ID3D11RasterizerState *m_rasterizerState = nullptr;
    D3DXMATRIX m_P;
    D3DXMATRIX m_M;
    D3DXMATRIX m_O;
};