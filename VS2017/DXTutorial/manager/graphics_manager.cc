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

#include "graphics_manager.h"

#include <sstream>
#include <string>

#include <core/system.h>
#include <entity/font.h>
#include <entity/model.h>
#include <manager/input_manager.h>

namespace naiive::manager {
void GraphicsManagerClass::Initialize(HWND hwnd, UINT width, UINT height) {
  /* Create SwapChain, Device and Context */
  InitializeDevice(hwnd, width, height);

  /* Create RenderTarget and DepthStencilState */
  InitializeOM(hwnd, width, height);

  /* Create RasterizerState */
  InitializeRasterizer(hwnd, width, height);

  float fov, aspect;
  fov = (float)DirectX::XM_PI / 3.0f;
  aspect = (float)width / (float)height;
  DirectX::XMStoreFloat4x4(
      &matrix_perspective_,
      DirectX::XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 1000.0f));

  DirectX::XMStoreFloat4x4(&matrix_orthogonal_,
                           DirectX::XMMatrixOrthographicLH(
                               (float)width, (float)height, 0.1f, 1000.0f));

  camera_.SetPos(0.0f, 2.0f, -10.0f);

  model_ = new naiive::entity::Model3D("res/sphere");
  model_->Initialize(device_);
  model_dup_.resize(1000);
  for (auto&& pos : model_dup_) {
    pos.x = distribution_xy_(rng_);
    pos.y = distribution_xy_(rng_);
    pos.z = distribution_z_(rng_);
  }

  gui_ = new naiive::entity::Font;
  gui_->Initialize(device_);

  light_.dir = DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f);
}
void GraphicsManagerClass::Shutdown() {
  if (gui_) {
    gui_->Shutdown();
    delete gui_;
    gui_ = nullptr;
  }
  if (model_) {
    model_->Shutdown();
    delete model_;
    model_ = nullptr;
  }
  ShutdownRasterizer();
  ShutdownOM();
  ShutdownDevice();
}
BOOL GraphicsManagerClass::OnUpdate() {
  DirectX::XMFLOAT4X4 view;
  float blend_factor[4] = {0, 0, 0, 0};
  BeginScene();
  camera_.GetMatrix(view);
  device_context_->OMSetRenderTargets(1, &render_target_view_,
                                      depth_stencil_view_);
  device_context_->OMSetDepthStencilState(depth_stencil_state_z_on_, 1);
  device_context_->OMSetBlendState(blend_state_alpha_off_, blend_factor,
                                   0xFFFFFFFF);

  ULONG total_models = static_cast<ULONG>(model_dup_.size());
  ULONG frustum_visible_models = 0UL;
  for (auto&& pos : model_dup_) {
    model_->MoveTo(pos);
    if (model_->Render(device_context_, view, matrix_perspective_,
                       camera_.GetPos(), light_.dir)) {
      ++frustum_visible_models;
    }
  }
  device_context_->OMSetDepthStencilState(depth_stencil_state_z_off_, 1);
  device_context_->OMSetBlendState(blend_state_alpha_on_, blend_factor,
                                   0xFFFFFFFF);
  LONG x, y;
  manager::InputManager().GetMouse(x, y);
  std::stringstream ss;
  ss << "MouseX: " << x << "\n";
  ss << "MouseY: " << y << "\n";
  ss << "FPS: " << core::System().Fps() << "\n";
  ss << "CPU: " << core::System().CpuUsage() << "%\n";
  ss << "Frustum Culling: " << frustum_visible_models << "/" << total_models
     << "\n";
  gui_->Render(device_, device_context_, ss.str(), {0, 1}, matrix_orthogonal_);
  EndScene();
  return TRUE;
}

void GraphicsManagerClass::InitializeDevice(HWND hwnd, UINT width,
                                            UINT height) {
  HRESULT hr = S_OK;

  IDXGIFactory* dxgi_factory = nullptr;  // To destroy
  hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgi_factory));
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get \"IDXGIFactory\" failed:", std::hex, std::uppercase, hr);

  IDXGIAdapter* dxgi_adapter = nullptr;  // To destroy
  hr = dxgi_factory->EnumAdapters(0, &dxgi_adapter);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get \"IDXGIAdapter\" failed:", std::hex, std::uppercase, hr);

  IDXGIOutput* dxgi_output = nullptr;  // To destroy
  hr = dxgi_adapter->EnumOutputs(0, &dxgi_output);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get \"IDXGIOutput\" failed:", std::hex, std::uppercase, hr);

  UINT num_dxgi_modes = 0U;
  hr = dxgi_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                       DXGI_ENUM_MODES_INTERLACED,
                                       &num_dxgi_modes, nullptr);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get mode number failed:", std::hex, std::uppercase, hr);
  std::vector<DXGI_MODE_DESC> mode_list(num_dxgi_modes);
  hr = dxgi_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                       DXGI_ENUM_MODES_INTERLACED,
                                       &num_dxgi_modes, &mode_list[0]);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get \"DXGI_MODE_DESC\" failed:", std::hex, std::uppercase, hr);

  DXGI_SWAP_CHAIN_DESC swap_chain_desc;
  ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));
  hr = E_FAIL;
  for (auto&& mode : mode_list) {
    if (mode.Width == static_cast<UINT>(width) &&
        mode.Height == static_cast<UINT>(height)) {
      swap_chain_desc.BufferDesc = mode;
      hr = S_OK;
      break;
    }
  }
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("No mode match with width", width, "and height", height);

  swap_chain_desc.SampleDesc.Count = 1;    // MSAA off
  swap_chain_desc.SampleDesc.Quality = 0;  // MSAA off
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount = 2;  // Double Buffer
  swap_chain_desc.OutputWindow = hwnd;
  swap_chain_desc.Windowed = TRUE;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swap_chain_desc.Flags = 0;  // No Advanced Flags

  D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_1;
  hr = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG,
      &feature_level, 1U, D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain_,
      &device_, nullptr, &device_context_);
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get \"ID3D11Device\",\"ID3D11DeviceContext\",\"IDXGISwapChain\" failed:",
   std::hex, std::uppercase, hr);

  hr = device_->QueryInterface(IID_PPV_ARGS(&debug_));
  ASSERT_MESSAGE(SUCCEEDED(hr))
  ("Get \"ID3D11Debug\" failed:", std::hex, std::uppercase, hr);

  SafeRelease(&dxgi_output);
  SafeRelease(&dxgi_adapter);
  SafeRelease(&dxgi_factory);
}
void GraphicsManagerClass::ShutdownDevice() {
  swap_chain_->SetFullscreenState(FALSE, nullptr);
  SafeRelease(&swap_chain_);
  SafeRelease(&device_context_);
  SafeRelease(&debug_);
  SafeRelease(&device_);
}

void GraphicsManagerClass::DebugDevice() {
  OutputDebugString("=====A=====\n");
  auto hr = debug_->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
  ASSERT(SUCCEEDED(hr));
  OutputDebugString("=====B=====\n");
}

void GraphicsManagerClass::InitializeOM(HWND hwnd, UINT width, UINT height) {
  UNREFERENCED_PARAMETER(hwnd);
  HRESULT hr = S_OK;

  /* Create RTV */
  ID3D11Texture2D* back_buffer;
  hr = swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
  ASSERT(SUCCEEDED(hr));
  hr = device_->CreateRenderTargetView(back_buffer, nullptr,
                                       &render_target_view_);
  SafeRelease(&back_buffer);
  ASSERT(SUCCEEDED(hr));

  /* Create DSV */
  ID3D11Texture2D* depth_stencil_buffer = nullptr;
  D3D11_TEXTURE2D_DESC depth_stencil_buffer_desc;
  ZeroMemory(&depth_stencil_buffer_desc, sizeof(depth_stencil_buffer_desc));
  depth_stencil_buffer_desc.Width = width;
  depth_stencil_buffer_desc.Height = height;
  depth_stencil_buffer_desc.MipLevels = 1;
  depth_stencil_buffer_desc.ArraySize = 1;
  depth_stencil_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_buffer_desc.SampleDesc.Count = 1;
  depth_stencil_buffer_desc.SampleDesc.Quality = 0;
  depth_stencil_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  depth_stencil_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depth_stencil_buffer_desc.CPUAccessFlags = 0;
  depth_stencil_buffer_desc.MiscFlags = 0;
  hr = device_->CreateTexture2D(&depth_stencil_buffer_desc, NULL,
                                &depth_stencil_buffer);
  ASSERT(SUCCEEDED(hr));
  D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
  ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));
  depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depth_stencil_view_desc.Texture2D.MipSlice = 0;
  hr = device_->CreateDepthStencilView(
      depth_stencil_buffer, &depth_stencil_view_desc, &depth_stencil_view_);
  SafeRelease(&depth_stencil_buffer);
  ASSERT(SUCCEEDED(hr));

  /* DepthStencilState */
  D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
  ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
  depth_stencil_desc.DepthEnable = TRUE;
  depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
  depth_stencil_desc.StencilEnable = TRUE;
  depth_stencil_desc.StencilReadMask = 0xFF;
  depth_stencil_desc.StencilWriteMask = 0xFF;
  /* Frontal face */
  depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  /* Back face */
  depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  hr = device_->CreateDepthStencilState(&depth_stencil_desc,
                                        &depth_stencil_state_z_on_);
  ASSERT(SUCCEEDED(hr));

  ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
  depth_stencil_desc.DepthEnable = FALSE;
  depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
  depth_stencil_desc.StencilEnable = TRUE;
  depth_stencil_desc.StencilReadMask = 0xFF;
  depth_stencil_desc.StencilWriteMask = 0xFF;
  /* Frontal face */
  depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  /* Back face */
  depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  hr = device_->CreateDepthStencilState(&depth_stencil_desc,
                                        &depth_stencil_state_z_off_);
  ASSERT(SUCCEEDED(hr));
  device_context_->OMSetDepthStencilState(depth_stencil_state_z_on_, 1);

  /* Blend State */
  D3D11_BLEND_DESC blend_desc;
  ZeroMemory(&blend_desc, sizeof(blend_desc));
  blend_desc.AlphaToCoverageEnable = FALSE;
  blend_desc.IndependentBlendEnable = FALSE;
  blend_desc.RenderTarget[0].BlendEnable = TRUE;
  blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
  blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blend_desc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
  hr = device_->CreateBlendState(&blend_desc, &blend_state_alpha_on_);
  ASSERT(SUCCEEDED(hr));
  blend_desc.RenderTarget[0].BlendEnable = FALSE;
  hr = device_->CreateBlendState(&blend_desc, &blend_state_alpha_off_);
  ASSERT(SUCCEEDED(hr));
}
void GraphicsManagerClass::ShutdownOM() {
  SafeRelease(&blend_state_alpha_off_);
  SafeRelease(&blend_state_alpha_on_);
  SafeRelease(&depth_stencil_state_z_off_);
  SafeRelease(&depth_stencil_state_z_on_);
  SafeRelease(&depth_stencil_view_);
  SafeRelease(&render_target_view_);
}

void GraphicsManagerClass::InitializeRasterizer(HWND hwnd, UINT width,
                                                UINT height) {
  UNREFERENCED_PARAMETER(hwnd);
  HRESULT hr = S_OK;

  /* State */
  D3D11_RASTERIZER_DESC resterizer_desc;
  ZeroMemory(&resterizer_desc, sizeof(resterizer_desc));
  resterizer_desc.FillMode = D3D11_FILL_SOLID;
  resterizer_desc.CullMode = D3D11_CULL_BACK;
  resterizer_desc.FrontCounterClockwise = FALSE;
  resterizer_desc.DepthBias = 0;
  resterizer_desc.DepthBiasClamp = 0.0f;
  resterizer_desc.SlopeScaledDepthBias = 0.0f;
  resterizer_desc.DepthClipEnable = TRUE;
  resterizer_desc.ScissorEnable = FALSE;
  resterizer_desc.MultisampleEnable = FALSE;
  resterizer_desc.AntialiasedLineEnable = FALSE;
  hr = device_->CreateRasterizerState(&resterizer_desc, &rasterizer_state_);
  ASSERT(SUCCEEDED(hr));
  device_context_->RSSetState(rasterizer_state_);

  /* ViewPort */
  D3D11_VIEWPORT viewport;
  ZeroMemory(&viewport, sizeof(viewport));
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;
  viewport.Width = (float)width;
  viewport.Height = (float)height;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  device_context_->RSSetViewports(1, &viewport);
}

void GraphicsManagerClass::ShutdownRasterizer() {
  SafeRelease(&rasterizer_state_);
}

GraphicsManagerClass& GraphicsManager() {
  static GraphicsManagerClass obj;
  return obj;
}
}  // namespace naiive::manager
