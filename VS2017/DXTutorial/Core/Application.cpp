#include <stdafx.h>

#include "Application.hpp"

#include <Manager/GraphicsManager.hpp>
#include <Manager/InputManager.hpp>
#include <Manager/SoundManager.hpp>

void Naiive::Core::ApplicationClass::Run(HINSTANCE hInst, INT nCmdShow) {
  HRESULT hr = S_OK;
  // TODO 1. Init Host
  // TODO 1.1 Memory, skip for now, using system alloced memory
  // TODO 1.2 CMD
  // TODO 1.3 View
  // TODO 1.4 Chase
  // TODO 1.5 Common
  // TODO 1.6 HostLocal->host.c
  // TODO 1.7 WAD
  // TODO 1.8 Keys
  // TODO 1.9 Console
  // TODO 1.10 Menu
  // TODO 1.11 EntiryDict
  // TODO 1.12 Model
  // TODO 1.13 Net(opt)
  // TODO 1.14 Server(opt)
  // TODO 1.15 Texture
  // TODO 1.16 Graphics(<- hWnd here)
  // TODO 1.17 Draw
  // TODO 1.18 Render R
  // TODO 1.19 Rasterizer D
  // TODO 1.20 Status Bar
  // TODO 1.21 ClientMainLoop
  // TODO 1.22 Input
  // TODO 2 HostMainLoop
  LoadString(hInst, IDS_APP_TITLE, m_AppTitle, MAX_LOADSTRING);
  LoadString(hInst, IDS_WINDOW_CLASS, m_WindowClass, MAX_LOADSTRING);
  InitializeWindowClass(hInst, nCmdShow);
  InitializeWindow(hInst, nCmdShow);
  Manager::InputManager().Initialize(m_hWnd, m_w, m_h);
  Manager::SoundManager().Initialize(m_hWnd, m_w, m_h);
  Manager::GraphicsManager().Initialize(m_hWnd, m_w, m_h);

  MSG msg;
  ZeroMemory(&msg, sizeof(MSG));
  while (msg.message != WM_QUIT) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    /* Input */
    if (!Manager::InputManager().OnUpdate()) {
      break;
    }
    if (Manager::InputManager().IsKeyDown(DIK_ESCAPE)) {
      break;
    }
    /* Graphics */
    if (!Manager::GraphicsManager().OnUpdate()) {
      break;
    }
  }

  Manager::GraphicsManager().Shutdown();
  Manager::SoundManager().Shutdown();
  Manager::InputManager().Shutdown();
  ShutdownWindow();
  ShutdownWindowClass();
}

LRESULT Naiive::Core::ApplicationClass::MessageHandler(HWND hWnd, UINT message,
                                                       WPARAM wParam,
                                                       LPARAM lParam) {
  switch (message) {
    default: {
      return Manager::GraphicsManager().MessageHandler(hWnd, message, wParam,
                                                       lParam);
    }
  }
  return 0;
}

LRESULT Naiive::Core::ApplicationClass::WinProc(HWND hWnd, UINT message,
                                                WPARAM wParam, LPARAM lParam) {
  auto hInst = HINSTANCE_FROM_HWND(hWnd);
  switch (message) {
    case WM_CREATE: {
      LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
      SetWindowLongPtr(
          hWnd, GWLP_USERDATA,
          reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
      break;
    }
    case WM_COMMAND: {
      int wmId = LOWORD(wParam);
      switch (wmId) {
        case IDM_ABOUT: {
          DialogBox(hInst, MAKEINTRESOURCE(IDD_NAIIVE_ABOUT), hWnd, About);
          break;
        }
        case IDM_EXIT: {
          PostQuitMessage(0);
          break;
        }
        default: {
          return Core::Application().MessageHandler(hWnd, message, wParam,
                                                    lParam);
        }
      }
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    default: {
      return Core::Application().MessageHandler(hWnd, message, wParam, lParam);
    }
  }
  return 0;
}

INT_PTR Naiive::Core::ApplicationClass::About(HWND hDlg, UINT message,
                                              WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
    case WM_INITDIALOG:
      return (INT_PTR)TRUE;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
      }
      break;
  }
  return (INT_PTR)FALSE;
}

void Naiive::Core::ApplicationClass::InitializeWindowClass(HINSTANCE hInst,
                                                           INT nCmdShow) {
  UNREFERENCED_PARAMETER(nCmdShow);
  HRESULT hr = S_OK;
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WinProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInst;
  wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NAIIVE_ICON));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCE(IDC_NAIIVE_MENU);
  wcex.lpszClassName = m_WindowClass;
  wcex.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NAIIVE_ICON_SMALL));
  hr = (RegisterClassEx(&wcex) == 0) ? E_FAIL : S_OK;
  FAILTHROW;
}

void Naiive::Core::ApplicationClass::ShutdownWindowClass() {
  auto hInst =
      reinterpret_cast<HINSTANCE>(GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE));
  UnregisterClass(m_WindowClass, hInst);
}

void Naiive::Core::ApplicationClass::InitializeWindow(HINSTANCE hInst,
                                                      INT nCmdShow) {
  HRESULT hr = S_OK;
  RECT paintRect{0, 0, static_cast<LONG>(m_w), static_cast<LONG>(m_h)};
  hr = AdjustWindowRect(&paintRect, WS_OVERLAPPEDWINDOW, false) ? S_OK : E_FAIL;
  FAILTHROW;
  m_hWnd = CreateWindow(m_WindowClass, m_AppTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, paintRect.right - paintRect.left,
                        paintRect.bottom - paintRect.top, nullptr, nullptr,
                        hInst, nullptr);
  hr = m_hWnd ? S_OK : E_FAIL;
  FAILTHROW;
  ShowWindow(m_hWnd, nCmdShow);
  UpdateWindow(m_hWnd);
}

void Naiive::Core::ApplicationClass::ShutdownWindow() {
  ShowCursor(true);
  DestroyWindow(m_hWnd);
  m_hWnd = nullptr;
}

Naiive::Core::ApplicationClass& Naiive::Core::Application(UINT w, UINT h) {
  static ApplicationClass obj(w, h);
  return obj;
}
