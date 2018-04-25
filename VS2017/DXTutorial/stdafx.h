// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#define FAILRETURN(hr) if((hr) != S_OK) {return (hr);}

template<class COM>
inline void SafeRelease(COM **ppCOM)
{
    if (*ppCOM != nullptr)
    {
        (*ppCOM)->Release();
        (*ppCOM) = nullptr;
    }
}
