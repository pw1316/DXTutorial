#ifndef __CORE_MESSAGE_HANDLER__
#define __CORE_MESSAGE_HANDLER__
#include <stdafx.h>

#include <map>
#include <set>

#include <Core/Interface/ICommand.hpp>
#include <Core/Interface/IView.hpp>
#include <Core/Message.hpp>

namespace Naiive::Core {
class MessageHandler {
 public:
  static MessageHandler* GetInstance() {
    if (g_instance == nullptr) {
      g_instance = new MessageHandler;
    }
    return g_instance;
  }

  static void ReleaseInstance() {
    if (g_instance != nullptr) {
      delete g_instance;
      g_instance = nullptr;
    }
  }

  void RegisterCommand(const std::string& name, ICommand* cmd) {
    auto iter = m_commandMap.find(name);
    if (iter != m_commandMap.end()) {
      delete iter->second;
    }
    m_commandMap[name] = cmd;
  }
  void RemoveCommand(const std::string& name) {
    auto iter = m_commandMap.find(name);
    if (iter != m_commandMap.end()) {
      delete iter->second;
      m_commandMap.erase(iter);
    }
  }

  void RegisterViewCommand(IView* view, const std::set<std::string>& list) {
    auto iter = m_viewCommandMap.find(view);
    if (iter != m_viewCommandMap.end()) {
      iter->second.insert(list.cbegin(), list.cend());
    } else {
      m_viewCommandMap[view] = list;
    }
  }
  void RemoveViewCommand(IView* view, const std::set<std::string>& list) {
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

  void ExecuteCommand(const Message& message) {
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
  BOOL HasCommand(const std::string& name) {
    return (m_commandMap.find(name) != m_commandMap.end());
  }

 private:
  static MessageHandler* g_instance;
  MessageHandler() = default;

  std::map<std::string, ICommand*>
      m_commandMap;  // ICommand's deletion handled HERE, NOT refer to its
                     // CREATOR
  std::map<IView*, std::set<std::string>>
      m_viewCommandMap;  // IView's deletion is handled by its CREATOR, NOT HERE
};
}  // namespace Naiive::Core
#endif
