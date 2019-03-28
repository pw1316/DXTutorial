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

#include <utils/debug.h>
#include <utils/slide_average.h>

namespace naiive::core {
class SystemClass {
  friend SystemClass& System();

 public:
  FLOAT GameTime() {
    QueryPerformanceCounter(&current_frame_time_);
    return 1.0f * (current_frame_time_.QuadPart - start_time_.QuadPart) /
           cpu_frequency_.QuadPart;
  }

  void CountFrame() {
    ++frame_count_since_last_frame_;
    QueryPerformanceCounter(&current_frame_time_);
    if (1.0f * (current_frame_time_.QuadPart - last_frame_time_.QuadPart) /
            cpu_frequency_.QuadPart >
        1.0f) {
      fps_slide_average_.Add(frame_count_since_last_frame_);
      frame_count_since_last_frame_ = 0UL;
      last_frame_time_ = current_frame_time_;

      if (cpu_readable_) {
        PDH_FMT_COUNTERVALUE cpu_counter_value;
        PdhCollectQueryData(cpu_hquery_);
        PdhGetFormattedCounterValue(cpu_hcounter, PDH_FMT_LONG, nullptr,
                                    &cpu_counter_value);
        cpu_usage_ = cpu_counter_value.longValue;
      }
    }
  }

  ULONG Fps() { return fps_slide_average_.Average(); }
  LONG get_cpu_usage() { return cpu_readable_ ? cpu_usage_ : 0L; }

  /* Debug DXGI */
  void ReportLiveObjects() {
    LOG(LOG_INFO)("=====A=====");
    dxgi_debug_->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    LOG(LOG_INFO)("=====B=====");
  }

 private:
  SystemClass() {
    QueryPerformanceFrequency(&cpu_frequency_);
    QueryPerformanceCounter(&start_time_);
    current_frame_time_ = last_frame_time_ = start_time_;

    PDH_STATUS status;
    status = PdhOpenQuery(NULL, 0, &cpu_hquery_);
    if (status != ERROR_SUCCESS) {
      cpu_readable_ = FALSE;
    }
    status = PdhAddCounter(cpu_hquery_,
                           TEXT("\\Processor(_Total)\\% processor time"), 0,
                           &cpu_hcounter);
    if (status != ERROR_SUCCESS) {
      cpu_readable_ = FALSE;
    }

    typedef HRESULT(WINAPI * funcdef)(const IID& iid, void** pp_debug);
    dxgi_debug_module_ = LoadLibrary("Dxgidebug.dll");
    ((funcdef)GetProcAddress(dxgi_debug_module_, "DXGIGetDebugInterface"))(
        IID_PPV_ARGS(&dxgi_debug_));
  }
  ~SystemClass() {
    SafeRelease(&dxgi_debug_);
    FreeLibrary(dxgi_debug_module_);
    if (cpu_readable_) {
      PdhCloseQuery(cpu_hquery_);
    }
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

  /* DXGI */
  HMODULE dxgi_debug_module_ = nullptr;
  IDXGIDebug* dxgi_debug_ = nullptr;
};  // namespace Naiive::Core

SystemClass& System();
}  // namespace naiive::core
#endif
