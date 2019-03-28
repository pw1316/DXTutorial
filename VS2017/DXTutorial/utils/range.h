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

#ifndef __UTILS_RANGE__
#define __UTILS_RANGE__

namespace naiive::utils {
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
    _Type operator*() { return value_; }

   private:
    _Type value_;
    _Type step_;
  };
  Range(_Type from, _Type to, _Type step) : from_(from), to_(to), step_(step) {}
  Range(_Type from, _Type to) : from_(from), to_(to), step_(1) {}
  Range(_Type to) : from_(0), to_(to), step_(1) {}
  Iterator begin() { return Iterator(from_, step_); }
  Iterator end() { return Iterator(to_, step_); }

 private:
  _Type from_;
  _Type to_;
  _Type step_;
};
}  // namespace naiive::utils
#endif
