#ifndef __CORE_SYSTEM__
#define __CORE_SYSTEM__
#include <stdafx.h>

#include <sstream>
#include <string>

namespace Naiive::Core {
class SystemClass {
  friend SystemClass& System();

 public:
  FLOAT GameTime() {
    QueryPerformanceCounter(&m_nowTime);
    return 1.0f * (m_nowTime.QuadPart - m_startTime.QuadPart) / m_freq.QuadPart;
  }

  template <class... _Args>
  void DebugInfo(_Args&&... args) {
    DebugLog("[INFO]", std::forward<_Args>(args)...);
  }
  template <class... _Args>
  void DebugWarn(_Args&&... args) {
    DebugLog("[WARN]", std::forward<_Args>(args)...);
  }
  template <class... _Args>
  void DebugError(_Args&&... args) {
    DebugLog("[ERROR]", std::forward<_Args>(args)...);
  }

 private:
  SystemClass() {
    QueryPerformanceFrequency(&m_freq);
    QueryPerformanceCounter(&m_startTime);
    m_nowTime = m_startTime;
  }

  template <class... _Args>
  void DebugLog(const char* logType, _Args&&... args) {
    std::stringstream ss;
    DebugLogToStream(ss, logType, std::forward<_Args>(args)...);
    ss << "\n";
    OutputDebugString(ss.str().c_str());
  }

  template <class _Stream, class _Arg, class... _Args>
  void DebugLogToStream(_Stream& ss, _Arg&& arg, _Args&&... args) {
    ss << arg;
    if constexpr (sizeof...(args) > 0) {
      DebugLogToStream(ss, std::forward<_Args>(args)...);
    }
  }
  LARGE_INTEGER m_freq;
  LARGE_INTEGER m_startTime;
  LARGE_INTEGER m_nowTime;

  CHAR m_debugText[1024];
};

SystemClass& System();
}  // namespace Naiive::Core
#endif
