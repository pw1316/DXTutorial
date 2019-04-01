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

#ifndef __CORE_SYSTEM__
#define __CORE_SYSTEM__
#include <sstream>
#include <string>

#include <Pdh.h>
#include <dxgidebug.h>

#include <utils/slide_average.h>

namespace naiive::core {
class SystemClass {
  friend SystemClass& System();
  static constexpr char kCpuQuery[] = "\\Processor(_Total)\\% processor time";

 public:
  void UpdateTime() {
    QueryPerformanceCounter(&current_frame_time_);  // Never fail
  }
  FLOAT GameTime();
  void CountFrame();

  ULONG Fps() { return fps_slide_average_.Average(); }
  LONG get_cpu_usage() { return cpu_readable_ ? cpu_usage_ : 0L; }

 private:
  SystemClass();
  ~SystemClass() {
    if (cpu_readable_) {
      PdhCloseQuery(cpu_hquery_);
    }
  }

  FLOAT DeltaTime(const LARGE_INTEGER& src, const LARGE_INTEGER& dst) {
    return 1.0f * (dst.QuadPart - src.QuadPart) / cpu_frequency_.QuadPart;
  }
  /* Timing */
  LARGE_INTEGER cpu_frequency_;
  LARGE_INTEGER start_time_;
  LARGE_INTEGER last_frame_time_;
  LARGE_INTEGER current_frame_time_;
  ULONG frame_count_since_last_frame_ = 0UL;
  utils::SlideAverage<ULONG> fps_slide_average_;

  /* CPU */
  BOOL cpu_readable_ = TRUE;
  HQUERY cpu_hquery_ = nullptr;
  HCOUNTER cpu_hcounter = nullptr;
  LONG cpu_usage_ = 0L;
};

SystemClass& System();
}  // namespace naiive::core
#endif
