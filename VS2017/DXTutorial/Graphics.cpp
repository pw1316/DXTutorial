#include "stdafx.h"
#include "Graphics.h"

HRESULT Graphics::Initialize(HWND hwnd, LONG w, LONG h)
{
    HRESULT hr;

    IDXGIFactory *factory;
    IDXGIAdapter *adapter;
    IDXGIOutput *adapterOutput;
    unsigned int numModes;
    DXGI_MODE_DESC *displayModeList;
    unsigned int numerator, denominator;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Texture2D* backBufferPtr;
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    D3D11_RASTERIZER_DESC rasterDesc;
    D3D11_VIEWPORT viewport;
    float fieldOfView, screenAspect;

    /* DX factory */
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    FAILRETURN(hr);
    /* DX adapter from factory */
    hr = factory->EnumAdapters(0, &adapter);
    FAILRETURN(hr);
    /* DX output from adapter */
    hr = adapter->EnumOutputs(0, &adapterOutput);
    FAILRETURN(hr);
    /* DX output with the mode */
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
    FAILRETURN(hr);
    displayModeList = new DXGI_MODE_DESC[numModes];
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
    FAILRETURN(hr);
    for (unsigned int i = 0; i < numModes; ++i)
    {
        if (displayModeList[i].Width == (unsigned int)w)
        {
            if (displayModeList[i].Height == (unsigned int)h)
            {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
                break;
            }
        }
    }
    delete[] displayModeList;
    displayModeList = 0;
    adapterOutput->Release();
    adapterOutput = 0;
    adapter->Release();
    adapter = 0;
    factory->Release();
    factory = 0;

    /* Initialize the swap chain description */
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    // Set to a single back buffer.
    swapChainDesc.BufferCount = 1;
    // Set the width and height of the back buffer.
    swapChainDesc.BufferDesc.Width = w;
    swapChainDesc.BufferDesc.Height = h;
    // Set regular 32-bit surface for the back buffer.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    // Set the usage of the back buffer.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    // Set the handle for the window to render to.
    swapChainDesc.OutputWindow = hwnd;
    // Turn multisampling off.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = true;
    // Set the scan line ordering and scaling to unspecified.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    // Discard the back buffer contents after presenting.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    // Don't set the advanced flags.
    swapChainDesc.Flags = 0;
    // Set the feature level to DirectX 11.
    featureLevel = D3D_FEATURE_LEVEL_11_0;
    // Create the swap chain, Direct3D device, and Direct3D device context.
    hr = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
        &featureLevel, 1,
        D3D11_SDK_VERSION,
        &swapChainDesc, &m_swapChain,
        &m_device,
        NULL,
        &m_deviceContext
    );
    FAILRETURN(hr);

    /* RTV on SwapChain */
    // Get the pointer to the back buffer.
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
    FAILRETURN(hr);
    // Create the render target view with the back buffer pointer.
    hr = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
    FAILRETURN(hr);
    // Release pointer to the back buffer as we no longer need it.
    backBufferPtr->Release();
    backBufferPtr = 0;

    // Initialize the description of the depth buffer.
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    // Set up the description of the depth buffer.
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
    // Create the texture for the depth buffer using the filled out description.
    hr = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    FAILRETURN(hr);

    // Initialize the description of the stencil state.
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;
    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // Create the depth stencil state.
    hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    FAILRETURN(hr);
    // Set the depth stencil state.
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

    // Initailze the depth stencil view.
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    // Create the depth stencil view.
    hr = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
    FAILRETURN(hr);
    // Bind the render target view and depth stencil buffer to the output render pipeline.
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    // Setup the raster description which will determine how and what polygons will be drawn.
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    // Create the rasterizer state from the description we just filled out.
    hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterizerState);
    FAILRETURN(hr);
    // Now set the rasterizer state.
    m_deviceContext->RSSetState(m_rasterizerState);

    // Setup the viewport for rendering.
    viewport.Width = (float)w;
    viewport.Height = (float)h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    // Create the viewport.
    m_deviceContext->RSSetViewports(1, &viewport);

    // Setup the projection matrix.
    fieldOfView = (float)D3DX_PI / 4.0f;
    screenAspect = (float)w / (float)h;
    // Create the projection matrix for 3D rendering.
    D3DXMatrixPerspectiveFovLH(&m_P, fieldOfView, screenAspect, 0.1f, 1000.0f);
    // Initialize the world matrix to the identity matrix.
    D3DXMatrixIdentity(&m_M);
    // Create an orthographic projection matrix for 2D rendering.
    D3DXMatrixOrthoLH(&m_O, (float)w, (float)h, 0.1f, 1000.0f);
    return S_OK;
}

HRESULT Graphics::Shutdown()
{
    if (m_rasterizerState)
    {
        m_rasterizerState->Release();
        m_rasterizerState = 0;
    }
    if (m_depthStencilView)
    {
        m_depthStencilView->Release();
        m_depthStencilView = 0;
    }
    if (m_depthStencilState)
    {
        m_depthStencilState->Release();
        m_depthStencilState = 0;
    }
    if (m_depthStencilBuffer)
    {
        m_depthStencilBuffer->Release();
        m_depthStencilBuffer = 0;
    }
    if (m_renderTargetView)
    {
        m_renderTargetView->Release();
        m_renderTargetView = 0;
    }
    if (m_deviceContext)
    {
        m_deviceContext->Release();
        m_deviceContext = 0;
    }
    if (m_device)
    {
        m_device->Release();
        m_device = 0;
    }
    if (m_swapChain)
    {
        m_swapChain->Release();
        m_swapChain = 0;
    }
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
    P = m_P;
}

void Graphics::GetWorldMatrix(D3DXMATRIX &M)
{
    M = m_M;
}

void Graphics::GetOrthoMatrix(D3DXMATRIX &O)
{
    O = m_O;
}

HRESULT Graphics::BeginScene(float r, float g, float b, float a)
{
    float color[4];
    // Setup the color to clear the buffer to.
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
    // Clear the back buffer.
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
    // Clear the depth buffer.
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    return S_OK;
}

HRESULT Graphics::EndScene()
{
    m_swapChain->Present(1, 0);
    return S_OK;
}
