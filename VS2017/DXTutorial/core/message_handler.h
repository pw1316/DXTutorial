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

#ifndef __CORE_MESSAGE_HANDLER__
#define __CORE_MESSAGE_HANDLER__
#include <unordered_map>
#include <unordered_set>

namespace naiive::core {
class CommandInterface;
class IView;
class Message;

class MessageHandlerClass {
  friend MessageHandlerClass& MessageHandler();
  using MessageSet = std::unordered_set<std::string>;

 public:
  void RegisterCommand(const std::string& name, CommandInterface* command);
  void RemoveCommand(const std::string& name);

  void RegisterViewCommand(IView* view, const MessageSet& list);
  void RemoveViewCommand(IView* view, const MessageSet& list);

  void ExecuteCommand(const Message& message);
  BOOL HasCommand(const std::string& name) {
    return command_map_.count(name) > 0;
  }

 private:
  MessageHandlerClass() = default;

  /* CommandInterface's deletion handled HERE, NOT refer to its CREATOR */
  std::unordered_map<std::string, CommandInterface*> command_map_;
  /* IView's deletion is handled by its CREATOR, NOT HERE */
  std::unordered_map<IView*, MessageSet> view_command_map_;
};

MessageHandlerClass& MessageHandler();
}  // namespace naiive::core
#endif
