#include <core/application.h>
#include <core/system.h>
#include <Utils/Debug.hpp>

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  auto&& sys = naiive::core::System();
  Debug(LOG_INFO)("System setup done at ", sys.GameTime());
  auto&& app = naiive::core::Application(1024, 768);
  app.Run(hInstance, nCmdShow);
  return 0;
}
