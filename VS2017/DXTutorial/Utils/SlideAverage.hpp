#ifndef __UTILS_SLIDE_AVERAGE__
#define __UTILS_SLIDE_AVERAGE__

namespace naiive::Utils {
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
}  // namespace Naiive::Utils
#endif
