#include <stdafx.h>
#include "GraphicsManager.hpp"

#include <vector>

void PW::Manager::GraphicsManager::Initialize(HWND hwnd, UINT w, UINT h)
{
    HRESULT hr = S_OK;
    /* Create SwapChain, Device and Context */
    InitializeDevice(hwnd, w, h);

    /* Create RenderTarget and DepthStencilState */
    InitializeOM(hwnd, w, h);

    /* Create RasterizerState */
    InitializeRasterizer(hwnd, w, h);

    float fov, aspect;
    fov = (float)D3DX_PI / 3.0f;
    aspect = (float)w / (float)h;
    D3DXMatrixPerspectiveFovLH(&m_MatrixProj, fov, aspect, 0.1f, 1000.0f);
    D3DXMatrixOrthoLH(&m_MatrixOrtho, (float)w, (float)h, 0.1f, 1000.0f);

    m_camera = new Camera;
    m_camera->SetPos(0.0f, 2.0f, -10.0f);

    m_model = new Entity::Model3D("Res/sphere");
    m_model->Initialize(m_device);

    m_gui = new Entity::Font;
    m_gui->Initialize(m_device);

    m_light = new Light;
    m_light->m_dir = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
}
void PW::Manager::GraphicsManager::Shutdown()
{
    if (m_light)
    {
        delete m_light;
        m_light = nullptr;
    }
    if (m_gui)
    {
        m_gui->Shutdown();
        delete m_gui;
        m_gui = nullptr;
    }
    if (m_model)
    {
        m_model->Shutdown();
        delete m_model;
        m_model = nullptr;
    }
    if (m_camera)
    {
        delete m_camera;
        m_camera = nullptr;
    }
    ShutdownRasterizer();
    ShutdownOM();
    ShutdownDevice();
}
void PW::Manager::GraphicsManager::OnRender()
{
    D3DXMATRIX view, proj = m_MatrixProj, ortho = m_MatrixOrtho;
    float blendFactor[4] = { 0,0,0,0 };
    BeginScene();
    m_camera->Render();
    m_camera->GetViewMatrix(view);
    m_deviceContext->OMSetDepthStencilState(m_DSStateWithZ, 1);
    m_deviceContext->OMSetBlendState(m_BlendStateWithoutAlpha, blendFactor, 0xFFFFFFFF);
    m_model->Render(m_deviceContext, view, proj, m_camera->GetPos(), m_light->m_dir);
    m_deviceContext->OMSetDepthStencilState(m_DSStateWithoutZ, 1);
    m_deviceContext->OMSetBlendState(m_BlendStateWithAlpha, blendFactor, 0xFFFFFFFF);
    m_gui->Render(m_device, m_deviceContext, "testaaaaaa", { 512, 384 }, ortho);
    EndScene();
}

void PW::Manager::GraphicsManager::GetRefreshRate(UINT w, UINT h, UINT &num, UINT &den)
{
    HRESULT hr = S_OK;
    /* Create DX factory */
    IDXGIFactory *factory;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    FAILTHROW;
    /* Create DX adapter from factory, Release factory */
    IDXGIAdapter *adapter;
    hr = factory->EnumAdapters(0, &adapter);
    SafeRelease(&factory);
    FAILTHROW;
    /* Create DX output from adapter, Release adapter */
    IDXGIOutput *adapterOutput;
    hr = adapter->EnumOutputs(0, &adapterOutput);
    SafeRelease(&adapter);
    FAILTHROW;
    /* Create DX mode from output, Release output */
    std::vector<DXGI_MODE_DESC> modeList;
    UINT numModes;
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
    FAILTHROW;
    modeList.resize(numModes);
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, &modeList[0]);
    SafeRelease(&adapterOutput);
    FAILTHROW;
    /* Get Refresh Rate */
    for (auto &mode : modeList)
    {
        if (mode.Width == static_cast<UINT>(w) && mode.Height == static_cast<UINT>(h))
        {
            num = mode.RefreshRate.Numerator;
            den = mode.RefreshRate.Denominator;
            break;
        }
    }
}

void PW::Manager::GraphicsManager::InitializeDevice(HWND hwnd, UINT w, UINT h)
{
    HRESULT hr = S_OK;
    UINT num = 0U, den = 0U;
    GetRefreshRate(w, h, num, den);
    /* Swap Chain */
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferDesc.Width = w;
    swapChainDesc.BufferDesc.Height = h;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = num;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = den;
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
        D3D11_CREATE_DEVICE_DEBUG,
        &featureLevel, 1,
        D3D11_SDK_VERSION,
        &swapChainDesc, &m_swapChain,
        &m_device,
        nullptr,
        &m_deviceContext
    );
    FAILTHROW;
}
void PW::Manager::GraphicsManager::ShutdownDevice()
{
    SafeRelease(&m_deviceContext);
    SafeRelease(&m_device);
    SafeRelease(&m_swapChain);
}

void PW::Manager::GraphicsManager::InitializeOM(HWND hwnd, UINT w, UINT h)
{
    HRESULT hr = S_OK;

    /* Create RTV */
    ID3D11Texture2D* backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    FAILTHROW;
    hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_RTView);
    SafeRelease(&backBuffer);
    FAILTHROW;

    /* Create DSV */
    ID3D11Texture2D *dsBuffer = nullptr;
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
    hr = m_device->CreateTexture2D(&depthBufferDesc, NULL, &dsBuffer);
    FAILTHROW;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    hr = m_device->CreateDepthStencilView(dsBuffer, &depthStencilViewDesc, &m_DSView);
    SafeRelease(&dsBuffer);
    FAILTHROW;

    /* RenderTarget */
    m_deviceContext->OMSetRenderTargets(1, &m_RTView, m_DSView);

    /* DepthStencilState */
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
    hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_DSStateWithZ);
    FAILTHROW;

    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    depthStencilDesc.DepthEnable = false;
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
    hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_DSStateWithoutZ);
    FAILTHROW;
    m_deviceContext->OMSetDepthStencilState(m_DSStateWithZ, 1);

    /* Blend State */
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
    hr = m_device->CreateBlendState(&blendDesc, &m_BlendStateWithAlpha);
    FAILTHROW;
    blendDesc.RenderTarget[0].BlendEnable = false;
    hr = m_device->CreateBlendState(&blendDesc, &m_BlendStateWithoutAlpha);
    FAILTHROW;
}
void PW::Manager::GraphicsManager::ShutdownOM()
{
    SafeRelease(&m_BlendStateWithoutAlpha);
    SafeRelease(&m_BlendStateWithAlpha);
    SafeRelease(&m_DSStateWithoutZ);
    SafeRelease(&m_DSStateWithZ);
    SafeRelease(&m_DSView);
    SafeRelease(&m_RTView);
}

void PW::Manager::GraphicsManager::InitializeRasterizer(HWND hwnd, UINT w, UINT h)
{
    HRESULT hr = S_OK;

    /* State */
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
    FAILTHROW;
    m_deviceContext->RSSetState(m_RState);

    /* ViewPort */
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(viewport));
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (float)w;
    viewport.Height = (float)h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);
}
void PW::Manager::GraphicsManager::ShutdownRasterizer()
{
    SafeRelease(&m_RState);
}
