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

#ifndef __UTILS_SLIDE_AVERAGE__
#define __UTILS_SLIDE_AVERAGE__

namespace naiive::utils {
template <class _Type>
class SlideAverage {
 public:
  void Reset() {
    tail_ = 0UL;
    cnt_ = 0UL;
    sum_ = 0;
  }
  void Add(_Type i) {
    _Type f = static_cast<_Type>(i);
    if (cnt_ < 8UL) {
      cache_[tail_] = i;
      tail_ = (tail_ + 1UL) & 7UL;
      ++cnt_;
      sum_ += i;
    } else {
      sum_ += i - cache_[tail_];
      cache_[tail_] = i;
      tail_ = (tail_ + 1UL) & 7UL;
    }
  }
  _Type Average() const { return cnt_ ? sum_ / cnt_ : 0; }

 private:
  _Type cache_[8];
  ULONG tail_ = 0;
  ULONG cnt_ = 0;
  _Type sum_ = 0;
};
}  // namespace naiive::utils
#endif
