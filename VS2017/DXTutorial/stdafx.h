#pragma once
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windows.h>
#include <assert.h>
#include <type_traits>

#define FAILRETURN() if(FAILED(hr)) {return E_FAIL;}

template<class COM>
inline void SafeRelease(COM **ppCOM)
{
    if (*ppCOM != nullptr)
    {
        (*ppCOM)->Release();
        (*ppCOM) = nullptr;
    }
}
