#include "stdafx.h"
#include "DXTutorial.hpp"

HINSTANCE PW::Core::System::g_hInst = nullptr;
PW::Core::System *PW::Core::System::g_instance = nullptr;

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = S_OK;
    PW::Core::System *sys = PW::Core::System::GetInstance(hInstance);
    if (sys == nullptr)
    {
        return -1;
    }
    sys->MainLoop();
    PW::Core::System::ReleaseInstance();
    return 0;
}
