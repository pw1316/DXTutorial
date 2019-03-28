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

#ifndef __CORE_MESSAGE__
#define __CORE_MESSAGE__
#include <string>

namespace naiive::core {
class Message {
 public:
  explicit Message(std::string name) : name_(name){};
  Message(std::string name, ULONG body) : name_(name), body_(body){};

  const std::string& get_name() const { return name_; }
  void set_name(const std::string& name) { name_ = name; }
  void set_name(std::string&& name) { name_ = std::move(name); }

  ULONG get_body() const { return body_; }
  void set_body(ULONG body) { body_ = body; }

  BOOL get_handled() const { return handled_; }
  void set_handled(BOOL handled) { handled_ = handled; }

 private:
  std::string name_;
  ULONG body_ = 0;
  BOOL handled_ = FALSE;
};
}  // namespace naiive::core
#endif
