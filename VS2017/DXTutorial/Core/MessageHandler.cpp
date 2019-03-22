#include <stdafx.h>

#include "MessageHandler.hpp"

#include <Core/Interface/ICommand.hpp>
#include <Core/Interface/IView.hpp>
#include <Core/Message.hpp>

namespace Naiive::Core {
void MessageHandlerClass::RegisterCommand(const std::string& name,
                                          ICommand* cmd) {
  if (m_commandMap.count(name)) {
    delete m_commandMap[name];
  }
  m_commandMap[name] = cmd;
}

void MessageHandlerClass::RemoveCommand(const std::string& name) {
  if (m_commandMap.count(name)) {
    delete m_commandMap[name];
    m_commandMap.erase(name);
  }
}

void MessageHandlerClass::RegisterViewCommand(IView* view, CRMessageSet list) {
  if (m_viewCommandMap.count(view)) {
    m_viewCommandMap[view].insert(list.begin(), list.end());
  } else {
    m_viewCommandMap[view] = list;
  }
}

void MessageHandlerClass::RemoveViewCommand(IView* view, CRMessageSet list) {
  if (m_viewCommandMap.count(view)) {
    for (auto&& cmd : list) {
      if (m_viewCommandMap[view].count(cmd)) {
        m_viewCommandMap[view].erase(cmd);
      }
      if (m_viewCommandMap[view].empty()) {
        m_viewCommandMap.erase(view);
      }
    }
  }
}

void MessageHandlerClass::ExecuteCommand(const Message& message) {
  /* Is command message, directly execute */
  auto&& name = message.GetName();
  if (m_commandMap.count(name)) {
    (*m_commandMap[name])(message);
  }
  /* Is view message, send to view */
  else {
    for (auto&& view : m_viewCommandMap) {
      if (view.second.count(name)) {
        view.first->OnMessage(message);
      }
    }
  }
}

MessageHandlerClass& Naiive::Core::MessageHandler() {
  static MessageHandlerClass obj;
  return obj;
}
}  // namespace Naiive::Core
