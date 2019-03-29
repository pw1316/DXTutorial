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

#include "application.h"

#include <core/system.h>
#include <manager/graphics_manager.h>
#include <manager/input_manager.h>
#include <manager/sound_manager.h>

#ifndef NAIIVE_NO_MENU
#define NAIIVE_NO_MENU 1
#endif

namespace naiive::core {
void ApplicationClass::Run(HINSTANCE hinstance, INT command_show) {
  HRESULT hr = S_OK;
  LoadString(hinstance, IDS_APP_TITLE, app_title_, kMaxLoadString);
  LoadString(hinstance, IDS_WINDOW_CLASS, window_class_, kMaxLoadString);
  InitializeWindowClass(hinstance, command_show);
  InitializeWindow(hinstance, command_show);
  manager::InputManager().Initialize(hwnd_, width_, height_);
  manager::SoundManager().Initialize(hwnd_, width_, height_);
  manager::GraphicsManager().Initialize(hwnd_, width_, height_);

  MSG msg;
  ZeroMemory(&msg, sizeof(MSG));
  while (msg.message != WM_QUIT) {
    System().CountFrame();
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
#if not NAIIVE_NO_MENU
      TranslateAccelerator(hwnd_, haccel_, &msg);
#endif
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    /* Input */
    if (!manager::InputManager().OnUpdate()) {
      break;
    }
    if (manager::InputManager().IsKeyDown(DIK_ESCAPE)) {
      break;
    }
    /* Graphics */
    if (!manager::GraphicsManager().OnUpdate()) {
      break;
    }
  }

  manager::GraphicsManager().Shutdown();
  manager::SoundManager().Shutdown();
  manager::InputManager().Shutdown();
  ShutdownWindow();
  ShutdownWindowClass();
}

void ApplicationClass::MessageHandler(HWND hwnd, UINT message, WPARAM wparam,
                                      LPARAM lparam) {
  manager::GraphicsManager().MessageHandler(hwnd, message, wparam, lparam);
}

LRESULT ApplicationClass::WinProc(HWND hwnd, UINT message, WPARAM wparam,
                                  LPARAM lparam) {
  auto hinstance = HinstanceFromHwnd(hwnd);
  switch (message) {
    case WM_CREATE: {
      auto create_struct = reinterpret_cast<LPCREATESTRUCT>(lparam);
      SetWindowLongPtr(
          hwnd, GWLP_USERDATA,
          reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
      break;
    }
    case WM_SYSCOMMAND: {
      switch (wparam & 0xFFF0) {
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
    case WM_NCRBUTTONDOWN: {
      return 0;
    }
#if not NAIIVE_NO_MENU
    case WM_COMMAND: {
      int command_id = LOWORD(wparam);
      switch (command_id) {
        case IDM_ABOUT: {
          auto dialog = CreateDialog(
              hinstance, MAKEINTRESOURCE(IDD_NAIIVE_ABOUT), hwnd, About);
          ShowWindow(dialog, SW_SHOW);
          break;
        }
        case IDM_EXIT: {
          PostQuitMessage(0);
          break;
        }
        default: {
          Application().MessageHandler(hwnd, message, wparam, lparam);
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
      Application().MessageHandler(hwnd, message, wparam, lparam);
      break;
    }
  }
  return DefWindowProc(hwnd, message, wparam, lparam);
}

INT_PTR ApplicationClass::About(HWND hwnd, UINT message, WPARAM wparam,
                                LPARAM lparam) {
  UNREFERENCED_PARAMETER(lparam);
  switch (message) {
    case WM_INITDIALOG:
      return (INT_PTR)TRUE;
    case WM_COMMAND:
      if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL) {
        EndDialog(hwnd, LOWORD(wparam));
        return (INT_PTR)TRUE;
      }
      break;
  }
  return (INT_PTR)FALSE;
}

void ApplicationClass::InitializeWindowClass(HINSTANCE hinstance,
                                             INT command_show) {
  UNREFERENCED_PARAMETER(command_show);
  HRESULT hr = S_OK;
  WNDCLASSEX wndclassex;
  wndclassex.cbSize = sizeof(WNDCLASSEX);
  wndclassex.style = CS_HREDRAW | CS_VREDRAW;
  wndclassex.lpfnWndProc = WinProc;
  wndclassex.cbClsExtra = 0;
  wndclassex.cbWndExtra = 0;
  wndclassex.hInstance = hinstance;
  wndclassex.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_NAIIVE_ICON));
  wndclassex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wndclassex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
#if NAIIVE_NO_MENU
  wndclassex.lpszMenuName = nullptr;
#else
  wndclassex.lpszMenuName = MAKEINTRESOURCE(IDC_NAIIVE_MENU);
#endif
  wndclassex.lpszClassName = window_class_;
  wndclassex.hIconSm =
      LoadIcon(hinstance, MAKEINTRESOURCE(IDI_NAIIVE_ICON_SMALL));
  hr = (RegisterClassEx(&wndclassex) == 0) ? E_FAIL : S_OK;
  FAILTHROW;
}

void ApplicationClass::ShutdownWindowClass() {
  auto hinstance = HinstanceFromHwnd(hwnd_);
  UnregisterClass(window_class_, hinstance);
}

void ApplicationClass::InitializeWindow(HINSTANCE hinstance, INT command_show) {
  HRESULT hr = S_OK;
  DWORD window_style =
      WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX ^ WS_MINIMIZEBOX;
  RECT paint_rect{0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
  hr = AdjustWindowRect(&paint_rect, window_style, false) ? S_OK : E_FAIL;
  FAILTHROW;
  hwnd_ = CreateWindow(window_class_, app_title_, window_style, 0, 0,
                       paint_rect.right - paint_rect.left,
                       paint_rect.bottom - paint_rect.top, nullptr, nullptr,
                       hinstance, nullptr);
  hr = hwnd_ ? S_OK : E_FAIL;
  FAILTHROW;
  ShowWindow(hwnd_, command_show);
  UpdateWindow(hwnd_);
#if not NAIIVE_NO_MENU
  haccel_ = LoadAccelerators(hinstance, MAKEINTRESOURCE(IDC_NAIIVE_ACCEL));
#endif
}

void ApplicationClass::ShutdownWindow() {
  ShowCursor(true);
#if not NAIIVE_NO_MENU
  DestroyAcceleratorTable(haccel_);
  haccel_ = nullptr;
#endif
  DestroyWindow(hwnd_);
  hwnd_ = nullptr;
}

ApplicationClass& Application(UINT width, UINT height) {
  static ApplicationClass obj(width, height);
  return obj;
}
}  // namespace naiive::core
