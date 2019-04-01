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

#include "input_manager.h"

namespace naiive::manager {
void InputManagerClass::Initialize(HWND hwnd, UINT width, UINT height) {
  HRESULT hr = S_OK;
  auto hinstance = HinstanceFromHwnd(hwnd);

  width_ = width;
  height_ = height;
  hr = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
                          (void**)&dinput_, nullptr);
  FAILTHROW;
  hr = dinput_->CreateDevice(GUID_SysKeyboard, &keyboard_device_, nullptr);
  FAILTHROW;
  hr = keyboard_device_->SetDataFormat(&c_dfDIKeyboard);
  FAILTHROW;
  hr = keyboard_device_->SetCooperativeLevel(
      hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
  FAILTHROW;
  hr = keyboard_device_->Acquire();
  FAILTHROW;

  hr = dinput_->CreateDevice(GUID_SysMouse, &mouse_device_, nullptr);
  FAILTHROW;
  hr = mouse_device_->SetDataFormat(&c_dfDIMouse);
  FAILTHROW;
  hr = mouse_device_->SetCooperativeLevel(
      hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
  FAILTHROW;
  hr = mouse_device_->Acquire();
  FAILTHROW;
}

void InputManagerClass::Shutdown() {
  mouse_device_->Unacquire();
  SafeRelease(&mouse_device_);
  keyboard_device_->Unacquire();
  SafeRelease(&keyboard_device_);
  SafeRelease(&dinput_);
}

BOOL InputManagerClass::OnUpdate() {
  HRESULT hr = S_OK;
  hr = keyboard_device_->GetDeviceState(sizeof(keyboard_state_),
                                        &keyboard_state_[0]);
  if (FAILED(hr)) {
    if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
      keyboard_device_->Acquire();
    } else {
      return FALSE;
    }
  }
  hr = mouse_device_->GetDeviceState(sizeof(DIMOUSESTATE),
                                     (LPVOID)&mouse_state_);
  if (FAILED(hr)) {
    if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
      mouse_device_->Acquire();
    } else {
      return FALSE;
    }
  }

  mouse_x_ += mouse_state_.lX;
  mouse_y_ += mouse_state_.lY;

  // if (mouse_x_ < 0) {
  //  mouse_x_ = 0;
  //}
  // if (mouse_y_ < 0) {
  //  mouse_y_ = 0;
  //}

  // if (mouse_x_ > width_) {
  //  mouse_x_ = width_;
  //}
  // if (mouse_y_ > height_) {
  //  mouse_y_ = height_;
  //}
  return TRUE;
}

InputManagerClass& InputManager() {
  static InputManagerClass obj;
  return obj;
}
}  // namespace naiive::manager
