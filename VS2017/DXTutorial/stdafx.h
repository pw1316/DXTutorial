#pragma once
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windows.h>
#include <assert.h>
#include <type_traits>

#define FAILTHROW if(FAILED(hr)) { throw 1; }

template<class COM>
inline void SafeRelease(COM **ppCOM)
{
    if (*ppCOM != nullptr)
    {
        (*ppCOM)->Release();
        (*ppCOM) = nullptr;
    }
}
