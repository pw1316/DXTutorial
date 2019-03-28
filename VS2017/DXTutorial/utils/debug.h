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

#ifndef __UTILS_DEBUG__
#define __UTILS_DEBUG__
#include <sstream>
#include <iostream>

namespace naiive::utils {
class DebugClass {
 public:
  enum class LogLevel : int { LOG_INFO = 0, LOG_WARN = 1, LOG_ERROR = 2 };
  DebugClass(const char* file_name, int line_number, LogLevel log_level)
      : file_name_(file_name),
        line_number_(line_number),
        log_level_(log_level) {}
  ~DebugClass() {
    std::stringstream ss;
    ss << "["
       << "IWE"[static_cast<int>(log_level_)] << "] " << file_name_ << " "
       << line_number_ << " " << ss_.str();
    OutputDebugString(ss.str().c_str());
    if (log_level_ == LogLevel::LOG_ERROR) {
      std::abort();
    }
  }

  template <class _Arg, class... _Args>
  void operator()(_Arg&& arg, _Args&&... args) {
    if constexpr (sizeof...(args) > 0) {
      ss_ << arg << " ";
      (*this)(std::forward<_Args>(args)...);
    } else {
      ss_ << arg << "\n";
    }
  }

 private:
  std::stringstream ss_;
  std::string file_name_;
  int line_number_;
  LogLevel log_level_;
};
}  // namespace naiive::utils

#ifdef NDEBUG
#define LOG(level)
#else
#define LOG(level)                              \
  naiive::utils::DebugClass(__FILE__, __LINE__, \
                            naiive::utils::DebugClass::LogLevel::level)
#endif
#define LOG_IF(level, expression) \
  if ((expression)) LOG(level)

#define CHECK(expression) \
  LOG_IF(LOG_ERROR, !(expression))("Check failed: " #expression " ")

#define CHECK_EQ(a, b) CHECK((a) == (b))
#endif
