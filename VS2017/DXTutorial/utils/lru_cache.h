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

#ifndef __UTILS_LRU_CACHE__
#define __UTILS_LRU_CACHE__
#include <unordered_map>

namespace naiive::utils {
template <class _Key, class _Value>
class LruCache {
  struct Node {
    _Key key;
    std::shared_ptr<_Value> p_value;
    Node* prev;
    Node* next;
  };

 public:
  LruCache(UINT capacity) : capacity_(capacity), size_(0U) {
    stencil_.prev = stencil_.next = &stencil_;
  }

  std::shared_ptr<_Value> Get(const _Key& key) {
    if (quick_.count(key)) {
      auto p = quick_[key];
      p->next->prev = p->prev;
      p->prev->next = p->next;
      p->next = stencil_.next;
      stencil_.next->prev = p;
      p->prev = &stencil_;
      stencil_.next = p;
    } else {
      if (size_ == capacity_) {
        Get(stencil_.prev->key);
        quick_.erase(stencil_.next->key);
      } else {
        auto np = new Node;
        np->prev = &stencil_;
        np->next = stencil_.next;
        stencil_.next = np;
        np->next->prev = np;
        ++size_;
      }
      stencil_.next->key = key;
      stencil_.next->p_value.reset(new _Value);
      quick_[key] = stencil_.next;
    }
    return quick_[key]->p_value;
  }

 private:
  Node stencil_;
  UINT capacity_;
  UINT size_;
  std::unordered_map<_Key, Node*> quick_;
};
}  // namespace naiive::utils
#endif
