#ifndef __STDAFX_H__
#define __STDAFX_H__
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define DIRECTINPUT_VERSION 0x0800
#define NAIIVE_NO_MENU 1

// Windows Header Files:
#include <Windows.h>
#include <assert.h>
#include <mmreg.h>
#include <type_traits>

#define FAILTHROW   \
  if (FAILED(hr)) { \
    throw 1;        \
  }

#define HINSTANCE_FROM_HWND(hWnd) \
  reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));

template <class COM>
inline void SafeRelease(COM** ppCOM) {
  if (*ppCOM != nullptr) {
    (*ppCOM)->Release();
    (*ppCOM) = nullptr;
  }
}
#endif  // !__STDAFX_H__
