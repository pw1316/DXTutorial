#pragma once
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#include "stdafx.h"

#include <DXGI.h>
#include <D3D11.h>
#include <D3DX10math.h>

#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"

class Graphics
{
public:
    Graphics() {}
    Graphics(const Graphics& rhs) {}
    Graphics(Graphics&& rhs) {}
    ~Graphics() {}

    HRESULT Initialize(HWND hwnd, LONG w, LONG h);
    void Shutdown();
    HRESULT OnRender();

    ID3D11Device *GetDevice();
    ID3D11DeviceContext *GetDeviceContext();

    void GetProjectionMatrix(D3DXMATRIX&);
    void GetWorldMatrix(D3DXMATRIX&);
    void GetOrthoMatrix(D3DXMATRIX&);
private:
    void BeginScene(float r, float g, float b, float a);
    HRESULT EndScene();

    IDXGISwapChain *m_swapChain = nullptr;
    ID3D11Device *m_device = nullptr;
    ID3D11DeviceContext *m_deviceContext = nullptr;
    ID3D11RenderTargetView *m_RTView = nullptr;
    ID3D11Texture2D *m_DSBuffer = nullptr;
    ID3D11DepthStencilState *m_DSState = nullptr;
    ID3D11DepthStencilView *m_DSView = nullptr;
    ID3D11RasterizerState *m_RState = nullptr;
    D3DXMATRIX m_MatrixProj{};
    D3DXMATRIX m_MatrixWorld{};
    D3DXMATRIX m_MatrixOrtho{};

    Camera *m_camera = nullptr;
    Mesh *m_mesh = nullptr;
    Shader *m_shader = nullptr;
};