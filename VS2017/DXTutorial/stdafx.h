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

#ifndef __STDAFX__
#define __STDAFX__
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define DIRECTINPUT_VERSION 0x0800

/* Windows Header Files: */
#include <Windows.h>

/* C libs */
#include <assert.h>

/* C++ libs */
#include <iostream>
#include <sstream>
#include <type_traits>
#include <unordered_map>

/* 3rdparty libs */
#include <mmreg.h>

#ifndef NDEBUG
#include <wm_map.h>
inline std::string GetWindowMessage(UINT message) {
  if (window_message_to_name.count(message)) {
    return window_message_to_name.at(message);
  }
  return std::to_string(message);
}
inline std::string GetSystemError() {
  CHAR buffer[256];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)buffer, 256, NULL);
  buffer[strlen(buffer) - 2] = 0;
  return buffer;
}
#else
inline std::string GetWindowMessage(UINT message) {
  UNREFERENCED_PARAMETER(message);
}
inline std::string GetSystemError() {}
#endif

#ifndef NDEBUG
namespace noaccess {
class DebugClass {
  typedef std::ios_base&(__cdecl ManipulatorType)(std::ios_base&);

 public:
  enum class LogLevel : int { LOG_INFO = 0, LOG_WARN = 1, LOG_ERROR = 2 };
  DebugClass(const char* file_name, int line_number, LogLevel log_level)
      : file_name_(file_name),
        line_number_(line_number),
        log_level_(log_level) {}
  ~DebugClass() {
    std::stringstream ss;
    ss << "IWE"[static_cast<int>(log_level_)] << " " << file_name_ << ":"
       << line_number_ << "] " << ss_.str();
    std::cout << ss.str();
    std::cerr << ss.str();
    if (log_level_ == LogLevel::LOG_ERROR) {
      std::abort();
    }
  }

  template <class... _Args>
  void operator()(_Args&&... args) {
    InnerLog(std::forward<_Args>(args)...);
  }

 private:
  template <class _Arg, class... _Args>
  void InnerLog(_Arg&& arg, _Args&&... args) {
    if constexpr (sizeof...(args) > 0) {
      ss_ << arg << " ";
      InnerLog(std::forward<_Args>(args)...);
    } else {
      ss_ << arg << "\n";
    }
  }

  // No space after manipulator
  template <class... _Args>
  void InnerLog(ManipulatorType* arg, _Args&&... args) {
    if constexpr (sizeof...(args) > 0) {
      ss_ << *arg;
      InnerLog(std::forward<_Args>(args)...);
    } else {
      ss_ << arg << "\n";
    }
  }

  std::stringstream ss_;
  std::string file_name_;
  int line_number_;
  LogLevel log_level_;
};
}  // namespace noaccess

#define LOG(level)                         \
  noaccess::DebugClass(__FILE__, __LINE__, \
                       noaccess::DebugClass::LogLevel::level)
#define LOG_IF(level, expression) \
  if ((expression)) LOG(level)
#else
#define LOG(level)
#define LOG_IF(level, expression)
#endif
#define CHECK(expression) LOG_IF(LOG_WARN, !(expression))
#define ASSERT(expression) \
  LOG_IF(LOG_ERROR, !(expression))("Assertion failed " #expression)
#define ASSERT_MESSAGE(expression) LOG_IF(LOG_ERROR, !(expression))

inline HINSTANCE HinstanceFromHwnd(HWND hwnd) {
  return reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
}

template <class COM>
inline void SafeRelease(COM** pp_com) {
  if (*pp_com != nullptr) {
    (*pp_com)->Release();
    (*pp_com) = nullptr;
  }
}
#endif  // !__STDAFX_H__
