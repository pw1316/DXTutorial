#ifndef __CORE_APPLICATION_H__
#define __CORE_APPLICATION_H__
#include <stdafx.h>

#include <Resource.h>

namespace Naiive::Core {
class IGraphics;

class ApplicationClass {
  friend ApplicationClass& Application(UINT w, UINT h);

 private:
  static constexpr UINT MAX_LOADSTRING = 100;

 public:
  void Run(HINSTANCE hInst, INT nCmdShow, IGraphics& graphics);

 private:
  ApplicationClass() = delete;
  ApplicationClass(UINT w, UINT h) : m_w(w), m_h(h) {}
  ~ApplicationClass() {}
  /* No copy, no move */
  ApplicationClass(const ApplicationClass& rhs) = delete;
  ApplicationClass(ApplicationClass&& rhs) = delete;
  ApplicationClass& operator=(const ApplicationClass& rhs) = delete;
  ApplicationClass& operator=(ApplicationClass&& rhs) = delete;

  static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
  static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam,
                                LPARAM lParam);
  void InitializeWindowClass(HINSTANCE hInst, INT nCmdShow);
  void ShutdownWindowClass();
  void InitializeWindow(HINSTANCE hInst, INT nCmdShow, IGraphics& graphics);
  void ShutdownWindow();

  CHAR m_AppTitle[MAX_LOADSTRING];
  CHAR m_WindowClass[MAX_LOADSTRING];
  HWND m_hWnd = nullptr;
  UINT m_w = 640;
  UINT m_h = 480;
};

ApplicationClass& Application(UINT w = 640U, UINT h = 480U);
}  // namespace Naiive::Core
#endif
