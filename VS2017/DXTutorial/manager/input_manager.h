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

#ifndef __MANAGER_INPUT_MANAGER__
#define __MANAGER_INPUT_MANAGER__
#include <algorithm>

#include <dinput.h>

#include <core/iview.h>

namespace naiive::manager {
class InputManagerClass : public core::IView {
  friend InputManagerClass& InputManager();

 private:
  static constexpr UINT kNumKeys = 256U;

 public:
  /* Override */
  virtual void Initialize(HWND hwnd, UINT width, UINT height) override;
  virtual void Shutdown() override;
  virtual BOOL OnUpdate() override;

  BOOL IsKeyDown(UINT key) {
    ASSERT(key < kNumKeys)("Invalid key");
    return keyboard_state_[key] & 0x80;
  }

  void GetMouse(LONG& x, LONG& y) {
    x = std::clamp(mouse_x_, 0L, width_);
    y = std::clamp(mouse_y_, 0L, height_);
  }

 private:
  InputManagerClass() {}
  virtual ~InputManagerClass() {}

  IDirectInput8* dinput_ = nullptr;
  IDirectInputDevice8* keyboard_device_ = nullptr;
  IDirectInputDevice8* mouse_device_ = nullptr;
  UCHAR keyboard_state_[kNumKeys];
  DIMOUSESTATE mouse_state_;
  LONG mouse_x_ = 0, mouse_y_ = 0;
  LONG width_ = 0, height_ = 0;
};

InputManagerClass& InputManager();
}  // namespace naiive::manager

#endif
