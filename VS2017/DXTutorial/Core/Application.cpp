#include "Application.hpp"

#include <Core/System.hpp>
#include <Manager/GraphicsManager.hpp>
#include <Manager/InputManager.hpp>
#include <Manager/SoundManager.hpp>

void Naiive::Core::ApplicationClass::Run(HINSTANCE hInst, INT nCmdShow) {
  HRESULT hr = S_OK;
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
    System().CountFrame();
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
#if not NAIIVE_NO_MENU
      TranslateAccelerator(m_hWnd, m_hAccel, &msg);
#endif
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

void Naiive::Core::ApplicationClass::MessageHandler(HWND hWnd, UINT message,
                                                    WPARAM wParam,
                                                    LPARAM lParam) {
  Manager::GraphicsManager().MessageHandler(hWnd, message, wParam, lParam);
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
    case WM_SYSCOMMAND: {
      switch (wParam & 0xFFF0) {
        case SC_MOVE:
        case SC_KEYMENU:
        case SC_MOUSEMENU: {
          return 0;
        }
        default:
          break;
      }
      break;
    }
    // case WM_NCLBUTTONDOWN:
    case WM_NCRBUTTONDOWN: {
      return 0;
    }
#if not NAIIVE_NO_MENU
    case WM_COMMAND: {
      int wmId = LOWORD(wParam);
      switch (wmId) {
        case IDM_ABOUT: {
          auto dialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_NAIIVE_ABOUT),
                                     hWnd, About);
          ShowWindow(dialog, SW_SHOW);
          break;
        }
        case IDM_EXIT: {
          PostQuitMessage(0);
          break;
        }
        default: {
          Application().MessageHandler(hWnd, message, wParam, lParam);
          break;
        }
      }
      break;
    }
#endif
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    default: {
      Application().MessageHandler(hWnd, message, wParam, lParam);
      break;
    }
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
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
#if NAIIVE_NO_MENU
  wcex.lpszMenuName = nullptr;
#else
  wcex.lpszMenuName = MAKEINTRESOURCE(IDC_NAIIVE_MENU);
#endif
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
  DWORD dwStyle =
      WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX ^ WS_MINIMIZEBOX;
  RECT paintRect{0, 0, static_cast<LONG>(m_w), static_cast<LONG>(m_h)};
  hr = AdjustWindowRect(&paintRect, dwStyle, false) ? S_OK : E_FAIL;
  FAILTHROW;
  m_hWnd = CreateWindow(m_WindowClass, m_AppTitle, dwStyle, 0, 0,
                        paintRect.right - paintRect.left,
                        paintRect.bottom - paintRect.top, nullptr, nullptr,
                        hInst, nullptr);
  hr = m_hWnd ? S_OK : E_FAIL;
  FAILTHROW;
  ShowWindow(m_hWnd, nCmdShow);
  UpdateWindow(m_hWnd);
#if not NAIIVE_NO_MENU
  m_hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_NAIIVE_ACCEL));
#endif
}

void Naiive::Core::ApplicationClass::ShutdownWindow() {
  ShowCursor(true);
#if not NAIIVE_NO_MENU
  DestroyAcceleratorTable(m_hAccel);
  m_hAccel = nullptr;
#endif
  DestroyWindow(m_hWnd);
  m_hWnd = nullptr;
}

Naiive::Core::ApplicationClass& Naiive::Core::Application(UINT w, UINT h) {
  static ApplicationClass obj(w, h);
  return obj;
}
