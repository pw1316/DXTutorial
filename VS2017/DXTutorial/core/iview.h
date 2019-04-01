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

#ifndef __CORE_IVIEW__
#define __CORE_IVIEW__
#include <set>
#include <string>

namespace naiive::core {
class Message;

class IView {
  // public Facade facade
  //{
  //    get{ return Facade.instance; }
  //}
 public:
  IView() = default;
  virtual ~IView() = default;

  /* No copy, no move */
  IView(const IView& rhs) = delete;
  IView(IView&& rhs) = delete;
  IView& operator=(const IView& rhs) = delete;
  IView& operator=(IView&& rhs) = delete;

  virtual void Initialize(HWND hwnd, UINT width, UINT height) = 0;
  virtual void Shutdown() = 0;
  virtual BOOL OnUpdate() = 0;
  virtual void OnMessage(const Message& message) {
    UNREFERENCED_PARAMETER(message);
  }
  virtual void CALLBACK MessageHandler(HWND hwnd, UINT message, WPARAM wparam,
                                       LPARAM lparam) {
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);
  }

 protected:
  std::set<std::string> messages_;
};
}  // namespace naiive::core
#endif
