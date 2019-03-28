#ifndef __UTILS_DEBUG__
#define __UTILS_DEBUG__
#include <sstream>

namespace naiive::Utils {
class DebugClass {
 public:
  enum class LogLevel : int { LOG_INFO = 0, LOG_WARN = 1, LOG_ERROR = 2 };
  DebugClass(const char* file, int line, LogLevel ll)
      : m_file(file), m_line(line), m_ll(ll) {}
  ~DebugClass() {
    std::stringstream ss;
    ss << "["
       << "IWE"[static_cast<int>(m_ll)] << "] " << m_file << " " << m_line
       << " " << m_ss.str();
    OutputDebugString(ss.str().c_str());
  }

  template <class _Arg, class... _Args>
  void operator()(_Arg&& arg, _Args&&... args) {
    if constexpr (sizeof...(args) > 0) {
      m_ss << arg << " ";
      (*this)(std::forward<_Args>(args)...);
    } else {
      m_ss << arg << "\n";
    }
  }

 private:
  std::stringstream m_ss;
  std::string m_file;
  LogLevel m_ll;
  int m_line;
};
}  // namespace Naiive::Utils

#ifdef NDEBUG
#define Debug(LEVEL) 
#else
#define Debug(LEVEL)                            \
  naiive::Utils::DebugClass(__FILE__, __LINE__, \
                            naiive::Utils::DebugClass::LogLevel::LEVEL)
#endif
#endif
