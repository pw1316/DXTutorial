#ifndef __STDAFX_H__
#define __STDAFX_H__
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <Windows.h>
#include <assert.h>
#include <type_traits>

#define FAILTHROW   \
  if (FAILED(hr)) { \
    throw 1;        \
  }

template <class COM>
inline void SafeRelease(COM** ppCOM) {
  if (*ppCOM != nullptr) {
    (*ppCOM)->Release();
    (*ppCOM) = nullptr;
  }
}
#endif  // !__STDAFX_H__
