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
  auto iter = m_viewCommandMap.find(view);
  if (iter != m_viewCommandMap.end()) {
    iter->second.insert(list.cbegin(), list.cend());
  } else {
    m_viewCommandMap[view] = list;
  }
}

void MessageHandlerClass::RemoveViewCommand(IView* view, CRMessageSet list) {
  auto iter = m_viewCommandMap.find(view);
  if (iter != m_viewCommandMap.end()) {
    for (auto cmd : list) {
      auto cmdIter = iter->second.find(cmd);
      if (cmdIter != iter->second.end()) {
        iter->second.erase(cmdIter);
      }
    }
    if (iter->second.empty()) {
      m_viewCommandMap.erase(iter);
    }
  }
}

void MessageHandlerClass::ExecuteCommand(const Message& message) {
  auto iter = m_commandMap.find(message.GetName());
  /* Is command message, directly execute */
  if (iter != m_commandMap.end()) {
    (*(iter->second))(message);
  }
  /* Is view message, send to view */
  else {
    for (auto view : m_viewCommandMap) {
      if (view.second.find(message.GetName()) != view.second.end()) {
        (*(view.first)).OnMessage(message);  // View must implement member
                                             // function: OnMessage(message)
      }
    }
  }
}

MessageHandlerClass& Naiive::Core::MessageHandler() {
  static MessageHandlerClass obj;
  return obj;
}
}  // namespace Naiive::Core
