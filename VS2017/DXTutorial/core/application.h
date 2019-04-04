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

#ifndef __CORE_APPLICATION__
#define __CORE_APPLICATION__
#include <Resource.h>

namespace naiive::core {
class ApplicationClass {
  friend ApplicationClass& Application(UINT width, UINT height);
  static constexpr UINT kMaxLoad = 100U;

 public:
  void Run(HINSTANCE hinstance, INT command_show);

 private:
  ApplicationClass(UINT width, UINT height) : width_(width), height_(height) {}
  ~ApplicationClass() {}
  /* No copy, no move */
  ApplicationClass(const ApplicationClass& rhs) = delete;
  ApplicationClass(ApplicationClass&& rhs) = delete;
  ApplicationClass& operator=(const ApplicationClass& rhs) = delete;
  ApplicationClass& operator=(ApplicationClass&& rhs) = delete;

  void CALLBACK MessageHandler(HWND hwnd, UINT message, WPARAM wparam,
                               LPARAM lparam);
  static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wparam,
                                  LPARAM lparam);
  void Initialize(HINSTANCE hinstance, INT command_show);
  void Shutdown();

  CHAR app_title_[kMaxLoad];
  CHAR window_class_[kMaxLoad];
  HINSTANCE hinstance_ = nullptr;
  HWND hwnd_ = nullptr;
  UINT width_ = 1024U;
  UINT height_ = 768U;
};

ApplicationClass& Application(UINT width = 1024U, UINT height = 768U);
}  // namespace naiive::core
#endif
