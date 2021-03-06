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
#include <utils/range.h>

namespace naiive::core {
void ApplicationClass::Run(HINSTANCE hinstance, INT command_show) {
  INT int_res = 0;

  int_res = LoadString(hinstance, IDS_APP_TITLE, app_title_, kMaxLoad);
  CHECK(int_res)("Missing app title");
  int_res = LoadString(hinstance, IDS_WINDOW_CLASS, window_class_, kMaxLoad);
  ASSERT_MESSAGE(int_res)("Missing window class name");
  Initialize(hinstance, command_show);
  manager::InputManager().Initialize(hwnd_, width_, height_);
  manager::SoundManager().Initialize(hwnd_, width_, height_);
  manager::GraphicsManager().Initialize(hwnd_, width_, height_);

  MSG msg;
  ZeroMemory(&msg, sizeof(MSG));
  BOOL is_quit = FALSE;
  while (!is_quit) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        is_quit = TRUE;
      }
    }
    // Input
    if (!manager::InputManager().OnUpdate()) {
      break;
    }
    // Sound position
    {
      FLOAT t = System().GameTime();
      FLOAT mt = t / sound_oval_T_;
      mt = (mt - std::floor(mt)) * DirectX::XM_2PI;
      FLOAT etl = 0, etr = DirectX::XM_2PI;
      for (auto i : utils::Range(50)) {
        FLOAT et = (etl + etr) * 0.5f;
        if (et - sound_oval_e_ * std::sin(et) > mt) {
          etr = et;
        } else {
          etl = et;
        }
      }
      FLOAT et = (etl + etr) * 0.5f;
      FLOAT thetat = std::tan(et * 0.5f);
      thetat *= std::sqrt((1 + sound_oval_e_) / (1 - sound_oval_e_));
      thetat = 2 * std::atan(thetat);
      if (thetat < 0.0f) {
        thetat += DirectX::XM_2PI;
      }
      FLOAT r = sound_oval_a_ * (1 - sound_oval_e_ * sound_oval_e_) /
                (1 + sound_oval_e_ * std::cos(thetat));
      sound_position_.x = r * std::sin(thetat);
      sound_position_.y = 0;
      sound_position_.z = -r * std::cos(thetat);
      manager::SoundManager().MoveTo(sound_position_.x, sound_position_.y,
                                     sound_position_.z);
    }
    // Sound
    if (manager::InputManager().IsKeyDown(DIK_S)) {
      manager::SoundManager().OnUpdate();
    }
    // Graphics
    manager::GraphicsManager().SoundMoveTo(sound_position_);
    if (!manager::GraphicsManager().OnUpdate()) {
      break;
    }
    System().CountFrame();
    if (manager::InputManager().IsKeyDown(DIK_ESCAPE)) {
      break;
    }
  }

  manager::GraphicsManager().Shutdown();
  manager::SoundManager().Shutdown();
  manager::InputManager().Shutdown();
  Shutdown();
}

LRESULT ApplicationClass::WinProc(HWND hwnd, UINT message, WPARAM wparam,
                                  LPARAM lparam) {
  switch (message) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    default: {
      manager::GraphicsManager().MessageHandler(hwnd, message, wparam, lparam);
      break;
    }
  }
  return DefWindowProc(hwnd, message, wparam, lparam);
}

void ApplicationClass::Initialize(HINSTANCE hinstance, INT command_show) {
  ATOM atom_res = 0;
  BOOL bool_res = TRUE;

  hinstance_ = hinstance;

  WNDCLASSEX wndclassex;
  wndclassex.cbSize = sizeof(WNDCLASSEX);
  wndclassex.style = CS_HREDRAW | CS_VREDRAW;
  wndclassex.lpfnWndProc = WinProc;
  wndclassex.cbClsExtra = 0;
  wndclassex.cbWndExtra = 0;
  wndclassex.hInstance = hinstance_;
  wndclassex.hIcon = LoadIcon(hinstance_, MAKEINTRESOURCE(IDI_NAIIVE_ICON));
  wndclassex.hCursor = nullptr;
  wndclassex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wndclassex.lpszMenuName = nullptr;
  wndclassex.lpszClassName = window_class_;
  wndclassex.hIconSm =
      LoadIcon(hinstance_, MAKEINTRESOURCE(IDI_NAIIVE_ICON_SMALL));
  atom_res = RegisterClassEx(&wndclassex);
  ASSERT(atom_res != 0);

  DWORD window_style = WS_OVERLAPPEDWINDOW;
  window_style ^= WS_THICKFRAME ^ WS_MAXIMIZEBOX ^ WS_MINIMIZEBOX;
  RECT paint_rect{0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
  bool_res = AdjustWindowRect(&paint_rect, window_style, false);
  ASSERT(bool_res);
  hwnd_ = CreateWindow(window_class_, app_title_, window_style, 0, 0,
                       paint_rect.right - paint_rect.left,
                       paint_rect.bottom - paint_rect.top, nullptr, nullptr,
                       hinstance_, nullptr);
  ASSERT(hwnd_ != nullptr);
  ShowWindow(hwnd_, command_show);
  UpdateWindow(hwnd_);
}

void ApplicationClass::Shutdown() {
  BOOL bool_res = TRUE;

  ShowCursor(TRUE);
  if (IsWindow(hwnd_)) {
    bool_res = DestroyWindow(hwnd_);
    ASSERT(bool_res);
  }
  hwnd_ = nullptr;

  bool_res = UnregisterClass(window_class_, hinstance_);
  ASSERT(bool_res);
  hinstance_ = nullptr;
}

ApplicationClass& Application(UINT width, UINT height) {
  static ApplicationClass obj(width, height);
  return obj;
}
}  // namespace naiive::core
