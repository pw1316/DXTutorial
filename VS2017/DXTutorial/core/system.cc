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

#include "system.h"

#include <utils/debug.h>

namespace naiive::core {
FLOAT SystemClass::GameTime() {
  UpdateTime();
  return DeltaTime(start_time_, current_frame_time_);
}

void SystemClass::CountFrame() {
  ++frame_count_since_last_frame_;
  UpdateTime();
  if (DeltaTime(last_frame_time_, current_frame_time_) > 1.0f) {
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

void SystemClass::ReportLiveObjects() {
  OutputDebugString("=====A=====\n");
  HRESULT hr = S_OK;
  hr = dxgi_debug_->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
  ASSERT(SUCCEEDED(hr))("Get DirectX objects failed");
  OutputDebugString("=====B=====\n");
}

SystemClass::SystemClass() {
  HRESULT hr = S_OK;
  QueryPerformanceFrequency(&cpu_frequency_);  // Never fail
  QueryPerformanceCounter(&start_time_);       // Never fail
  current_frame_time_ = last_frame_time_ = start_time_;

  hr = PdhOpenQuery(nullptr, 0, &cpu_hquery_);
  LOG_IF(LOG_WARN, FAILED(hr))("Open PDH query failed");
  if (FAILED(hr)) {
    cpu_readable_ = FALSE;
  } else {
    hr = PdhAddCounter(cpu_hquery_, kCpuQuery, 0, &cpu_hcounter);
    LOG_IF(LOG_WARN, FAILED(hr))("Add PDH counter failed");
    if (FAILED(hr)) {
      cpu_readable_ = FALSE;
    }
  }

  typedef HRESULT(WINAPI * funcdef)(const IID& iid, void** pp_debug);
  dxgi_debug_module_ = LoadLibrary("Dxgidebug.dll");
  hr = dxgi_debug_module_ == nullptr ? E_FAIL : S_OK;
  ASSERT(SUCCEEDED(hr))("Load module \"Dxgidebug.dll\" failed");

  auto dxgi_debug_function =
      (funcdef)GetProcAddress(dxgi_debug_module_, "DXGIGetDebugInterface");
  hr = dxgi_debug_function == nullptr ? E_FAIL : S_OK;
  ASSERT(SUCCEEDED(hr))("Get address of \"DXGIGetDebugInterface\" failed");
  hr = dxgi_debug_function(IID_PPV_ARGS(&dxgi_debug_));
  ASSERT(SUCCEEDED(hr))("Get \"IDXGIDebug\" failed");
}

SystemClass& System() {
  static SystemClass obj;
  return obj;
}
}  // namespace naiive::core
