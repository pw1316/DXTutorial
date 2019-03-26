#ifndef __CORE_SYSTEM__
#define __CORE_SYSTEM__
#include <stdafx.h>

#include <sstream>
#include <string>

#include <dxgidebug.h>

#include <Utils/Debug.hpp>
#include <Utils/SlideAverage.hpp>

namespace Naiive::Core {
class SystemClass {
  friend SystemClass& System();

 public:
  FLOAT GameTime() {
    QueryPerformanceCounter(&m_nowTime);
    return 1.0f * (m_nowTime.QuadPart - m_startTime.QuadPart) / m_freq.QuadPart;
  }

  void CountFrame() {
    ++m_frameCount;
    GameTime();
    if (1.0f * (m_nowTime.QuadPart - m_lastTime.QuadPart) / m_freq.QuadPart >
        1.0f) {
      m_sa.Add(m_frameCount);
      m_frameCount = 0UL;
      m_lastTime = m_nowTime;
    }
  }

  ULONG GetFPS() { return m_sa.Average(); }

  /* Debug DXGI */
  void ReportLiveObjects() {
    Debug(LOG_INFO)("=====A=====");
    m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    Debug(LOG_INFO)("=====B=====");
  }

 private:
  SystemClass() {
    QueryPerformanceFrequency(&m_freq);
    QueryPerformanceCounter(&m_startTime);
    m_nowTime = m_lastTime = m_startTime;
    typedef HRESULT(WINAPI * funcdef)(const IID& riid, void** ppDebug);
    m_dxgiDebugModule = LoadLibrary("Dxgidebug.dll");
    ((funcdef)GetProcAddress(m_dxgiDebugModule, "DXGIGetDebugInterface"))(
        IID_PPV_ARGS(&m_dxgiDebug));
  }
  ~SystemClass() {
    SafeRelease(&m_dxgiDebug);
    FreeLibrary(m_dxgiDebugModule);
  }
  LARGE_INTEGER m_freq;
  LARGE_INTEGER m_startTime;
  LARGE_INTEGER m_lastTime;
  LARGE_INTEGER m_nowTime;
  ULONG m_frameCount = 0UL;
  Utils::SlideAverage<ULONG> m_sa;

  HMODULE m_dxgiDebugModule = nullptr;
  IDXGIDebug* m_dxgiDebug = nullptr;
};

SystemClass& System();
}  // namespace Naiive::Core
#endif
