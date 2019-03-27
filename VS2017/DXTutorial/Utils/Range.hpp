#ifndef __UTILS_RANGE__
#define __UTILS_RANGE__
#include <stdafx.h>

namespace Naiive::Utils {
template <class _Type>
class Range {
 public:
  class Iterator {
   public:
    Iterator(_Type value, _Type step) : value_(value), step_(step) {}
    bool operator==(const Iterator& rhs) const {
      return step_ > 0 ? value_ >= rhs.value_ : value_ <= rhs.value_;
    }
    bool operator!=(const Iterator& rhs) const {
      return step_ > 0 ? value_ < rhs.value_ : value_ > rhs.value_;
    }
    Iterator& operator++() {
      value_ += step_;
      return *this;
    }
    Iterator operator++(int) {
      auto ri = *this;
      value_ += step_;
      return ri;
    }
    int operator*() { return value_; }

   private:
    _Type value_;
    _Type step_;
  };
  Range(_Type from, _Type to, _Type step)
      : start_(from), end_(to), step_(step) {}
  Range(_Type from, _Type to) : start_(from), end_(to), step_(1) {}
  Range(_Type to) : start_(0), end_(to), step_(1) {}
  Iterator begin() { return Iterator(start_, step_); }
  Iterator end() { return Iterator(end_, step_); }

 private:
  _Type start_;
  _Type end_;
  _Type step_;
};
}  // namespace Naiive::Utils
#endif
