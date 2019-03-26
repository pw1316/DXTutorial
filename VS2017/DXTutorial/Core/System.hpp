#ifndef __CORE_SYSTEM__
#define __CORE_SYSTEM__
#include <stdafx.h>

#include <sstream>
#include <string>

#include <Pdh.h>
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

      if (m_canReadCpu) {
        PDH_FMT_COUNTERVALUE value;
        PdhCollectQueryData(m_queryHandle);
        PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, NULL,
                                    &value);
        m_cpuUsage = value.longValue;
      }
    }
  }

  ULONG GetFPS() { return m_sa.Average(); }
  long GetCPU() { return m_canReadCpu ? m_cpuUsage : 0; }

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

    PDH_STATUS status;
    m_canReadCpu = true;
    status = PdhOpenQuery(NULL, 0, &m_queryHandle);
    if (status != ERROR_SUCCESS) {
      m_canReadCpu = false;
    }
    status = PdhAddCounter(m_queryHandle,
                           TEXT("\\Processor(_Total)\\% processor time"), 0,
                           &m_counterHandle);
    if (status != ERROR_SUCCESS) {
      m_canReadCpu = false;
    }
    m_lastSampleTime = GetTickCount();
    m_cpuUsage = 0;

    typedef HRESULT(WINAPI * funcdef)(const IID& riid, void** ppDebug);
    m_dxgiDebugModule = LoadLibrary("Dxgidebug.dll");
    ((funcdef)GetProcAddress(m_dxgiDebugModule, "DXGIGetDebugInterface"))(
        IID_PPV_ARGS(&m_dxgiDebug));
  }
  ~SystemClass() {
    SafeRelease(&m_dxgiDebug);
    FreeLibrary(m_dxgiDebugModule);
    if (m_canReadCpu) {
      PdhCloseQuery(m_queryHandle);
    }
  }
  /* Timing */
  LARGE_INTEGER m_freq;
  LARGE_INTEGER m_startTime;
  LARGE_INTEGER m_lastTime;
  LARGE_INTEGER m_nowTime;
  ULONG m_frameCount = 0UL;
  Utils::SlideAverage<ULONG> m_sa;

  /* CPU */
  bool m_canReadCpu;
  HQUERY m_queryHandle;
  HCOUNTER m_counterHandle;
  unsigned long m_lastSampleTime;
  long m_cpuUsage;

  /* DXGI */
  HMODULE m_dxgiDebugModule = nullptr;
  IDXGIDebug* m_dxgiDebug = nullptr;
};  // namespace Naiive::Core

SystemClass& System();
}  // namespace Naiive::Core
#endif
