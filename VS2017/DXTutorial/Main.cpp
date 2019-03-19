#include <stdafx.h>

#include <Core/Application.hpp>
#include <Core/System.hpp>
#include <Manager/GraphicsManager.hpp>
//
//HINSTANCE Naiive::Temp::System::g_hInst = nullptr;
//Naiive::Temp::System* Naiive::Temp::System::g_instance = nullptr;

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  auto&& sys = Naiive::Core::System();
  sys.DebugInfo("System setup done at ", sys.GameTime());
  auto&& app = Naiive::Core::Application(1024, 768);
  auto&& g = Naiive::Manager::GraphicsManager();
  app.Run(hInstance, nCmdShow, g);
  return 0;
}
