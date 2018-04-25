#include "stdafx.h"
#include "Graphics.h"

#include <vector>

HRESULT Graphics::Initialize(HWND hwnd, LONG w, LONG h)
{
    HRESULT hr;

    /* Create DX factory */
    IDXGIFactory *factory;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    FAILRETURN(hr);
    /* Create DX adapter from factory, Release factory */
    IDXGIAdapter *adapter;
    hr = factory->EnumAdapters(0, &adapter);
    SafeRelease(&factory);
    FAILRETURN(hr);
    /* Create DX output from adapter, Release adapter */
    IDXGIOutput *adapterOutput;
    hr = adapter->EnumOutputs(0, &adapterOutput);
    SafeRelease(&adapter);
    FAILRETURN(hr);
    /* Create DX mode from output, Release output */
    std::vector<DXGI_MODE_DESC> modeList;
    UINT numModes;
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
    /* UGLY!!! */
    if (FAILED(hr))
    {
        SafeRelease(&adapterOutput);
        return hr;
    }
    modeList.resize(numModes);
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, &modeList[0]);
    SafeRelease(&adapterOutput);
    FAILRETURN(hr);
    /* Get Refresh Rate */
    UINT numerator, denominator;
    for (auto &mode : modeList)
    {
        if (mode.Width == static_cast<UINT>(w) && mode.Height == static_cast<UINT>(h))
        {
            numerator = mode.RefreshRate.Numerator;
            denominator = mode.RefreshRate.Denominator;
            break;
        }
    }

    /* Swap Chain */
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferDesc.Width = w;
    swapChainDesc.BufferDesc.Height = h;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = 1;// MSAA off
    swapChainDesc.SampleDesc.Quality = 0;// MSAA off
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;// Single Buffer
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;// No Advanced Flags
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        &featureLevel, 1,
        D3D11_SDK_VERSION,
        &swapChainDesc, &m_swapChain,
        &m_device,
        nullptr,
        &m_deviceContext
    );
    FAILRETURN(hr);
    /* Create RTV on SwapChain */
    ID3D11Texture2D* backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    FAILRETURN(hr);
    hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_RTView);
    SafeRelease(&backBuffer);
    FAILRETURN(hr);

    /* Depth Buffer */
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = w;
    depthBufferDesc.Height = h;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;
    hr = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_DSBuffer);
    FAILRETURN(hr);

    /* Depth Stencil State */
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;
    /* Frontal face */
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    /* Back face */
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_DSState);
    FAILRETURN(hr);
    m_deviceContext->OMSetDepthStencilState(m_DSState, 1);

    /* Depth Stencil View */
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    hr = m_device->CreateDepthStencilView(m_DSBuffer, &depthStencilViewDesc, &m_DSView);
    FAILRETURN(hr);

    /* Bind RTV and DSV */
    m_deviceContext->OMSetRenderTargets(1, &m_RTView, m_DSView);

    /* Rasterize State */
    D3D11_RASTERIZER_DESC resterizeState;
    ZeroMemory(&resterizeState, sizeof(resterizeState));
    resterizeState.FillMode = D3D11_FILL_SOLID;
    resterizeState.CullMode = D3D11_CULL_BACK;
    resterizeState.FrontCounterClockwise = false;
    resterizeState.DepthBias = 0;
    resterizeState.DepthBiasClamp = 0.0f;
    resterizeState.SlopeScaledDepthBias = 0.0f;
    resterizeState.DepthClipEnable = true;
    resterizeState.ScissorEnable = false;
    resterizeState.MultisampleEnable = false;
    resterizeState.AntialiasedLineEnable = false;
    hr = m_device->CreateRasterizerState(&resterizeState, &m_RState);
    FAILRETURN(hr);
    m_deviceContext->RSSetState(m_RState);

    /* Viewport */
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(viewport));
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (float)w;
    viewport.Height = (float)h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);

    float fov, aspect;
    fov = (float)D3DX_PI / 4.0f;
    aspect = (float)w / (float)h;
    D3DXMatrixPerspectiveFovLH(&m_MatrixProj, fov, aspect, 0.1f, 1000.0f);
    D3DXMatrixIdentity(&m_MatrixWorld);
    D3DXMatrixOrthoLH(&m_MatrixOrtho, (float)w, (float)h, 0.1f, 1000.0f);
    return S_OK;
}

HRESULT Graphics::Shutdown()
{
    SafeRelease(&m_RState);
    SafeRelease(&m_DSView);
    SafeRelease(&m_DSState);
    SafeRelease(&m_DSBuffer);
    SafeRelease(&m_RTView);
    SafeRelease(&m_deviceContext);
    SafeRelease(&m_device);
    SafeRelease(&m_swapChain);
    return S_OK;
}

HRESULT Graphics::OnRender()
{
    BeginScene(0.7f, 0.2f, 0.1f, 0.0f);
    EndScene();
    return S_OK;
}

ID3D11Device *Graphics::GetDevice()
{
    return m_device;
}

ID3D11DeviceContext *Graphics::GetDeviceContext()
{
    return m_deviceContext;
}

void Graphics::GetProjectionMatrix(D3DXMATRIX &P)
{
    P = m_MatrixProj;
}

void Graphics::GetWorldMatrix(D3DXMATRIX &M)
{
    M = m_MatrixWorld;
}

void Graphics::GetOrthoMatrix(D3DXMATRIX &O)
{
    O = m_MatrixOrtho;
}

HRESULT Graphics::BeginScene(float r, float g, float b, float a)
{
    float color[] = { r, g, b, a };
    m_deviceContext->ClearRenderTargetView(m_RTView, color);
    m_deviceContext->ClearDepthStencilView(m_DSView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    return S_OK;
}

HRESULT Graphics::EndScene()
{
    m_swapChain->Present(1, 0);
    return S_OK;
}
