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

#include "message_handler.h"

#include <core/command_interface.h>
#include <core/iview.h>
#include <core/message.h>

namespace naiive::core {
void MessageHandlerClass::RegisterCommand(const std::string& name,
                                          CommandInterface* command) {
  if (command_map_.count(name)) {
    delete command_map_[name];
  }
  command_map_[name] = command;
}

void MessageHandlerClass::RemoveCommand(const std::string& name) {
  if (command_map_.count(name)) {
    delete command_map_[name];
    command_map_.erase(name);
  }
}

void MessageHandlerClass::RegisterViewCommand(IView* view,
                                              const MessageSet& list) {
  if (view_command_map_.count(view)) {
    view_command_map_[view].insert(list.begin(), list.end());
  } else {
    view_command_map_[view] = list;
  }
}

void MessageHandlerClass::RemoveViewCommand(IView* view,
                                            const MessageSet& list) {
  if (view_command_map_.count(view)) {
    for (auto&& cmd : list) {
      if (view_command_map_[view].count(cmd)) {
        view_command_map_[view].erase(cmd);
      }
      if (view_command_map_[view].empty()) {
        view_command_map_.erase(view);
      }
    }
  }
}

void MessageHandlerClass::ExecuteCommand(const Message& message) {
  /* Is command message, directly execute */
  auto&& name = message.get_name();
  if (command_map_.count(name)) {
    (*command_map_[name])(message);
  }
  /* Is view message, send to view */
  else {
    for (auto&& view : view_command_map_) {
      if (view.second.count(name)) {
        view.first->OnMessage(message);
      }
    }
  }
}

MessageHandlerClass& MessageHandler() {
  static MessageHandlerClass obj;
  return obj;
}
}  // namespace naiive::core
