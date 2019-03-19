#include <stdafx.h>

#include <Core/Application.hpp>
#include <Core/System.hpp>
#include <Manager/GraphicsManager.hpp>

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  auto&& sys = Naiive::Core::System();
  sys.DebugInfo("System setup done at ", sys.GameTime());
  auto&& app = Naiive::Core::Application(1024, 768);
  app.Run(hInstance, nCmdShow, Naiive::Manager::GraphicsManager());
  return 0;
}
