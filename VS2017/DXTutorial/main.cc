/* MIT License

Copyright (c) 2018 Joker Yough

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/

#include <core/application.h>
#include <core/system.h>

int APIENTRY WinMain(_In_ HINSTANCE hinstance,
                     _In_opt_ HINSTANCE previous_hinstance,
                     _In_ LPSTR command_line, _In_ int command_show) {
  UNREFERENCED_PARAMETER(previous_hinstance);
  UNREFERENCED_PARAMETER(command_line);

  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  auto&& sys = naiive::core::System();
  LOG(LOG_INFO)("System setup done at", sys.GameTime());
  auto&& app = naiive::core::Application(1024, 768);
  app.Run(hinstance, command_show);
  LOG(LOG_INFO)("Press ENTER to close this window . . .");
#ifdef _DEBUG
  std::getchar();
  FreeConsole();
#endif;
  return 0;
}
