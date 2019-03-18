#include <stdafx.h>

#include <Core/Application.hpp>
#include <Core/System.hpp>
#include "DXTutorial.hpp"

HINSTANCE PW::Core::System::g_hInst = nullptr;
PW::Core::System* PW::Core::System::g_instance = nullptr;

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  auto&& sys = Naiive::Core::System();
  sys.DebugInfo("System setup done at ", sys.GameTime());
  auto&& app = Naiive::Core::Application(1024, 768);

  PW::Core::System* sys2 = PW::Core::System::GetInstance(hInstance);
  if (sys2 == nullptr) {
    return -1;
  }
  sys2->MainLoop();
  PW::Core::System::ReleaseInstance();
  return 0;
}
