#ifndef __CORE_MESSAGE_HANDLER__
#define __CORE_MESSAGE_HANDLER__
#include <stdafx.h>

#include <unordered_map>
#include <unordered_set>

namespace Naiive::Core {
class ICommand;
class IView;
class Message;

class MessageHandlerClass {
  friend MessageHandlerClass& MessageHandler();
  using MessageSet = std::unordered_set<std::string>;
  using CRMessageSet = const MessageSet&;
 public:
  void RegisterCommand(const std::string& name, ICommand* cmd);
  void RemoveCommand(const std::string& name);

  void RegisterViewCommand(IView* view, CRMessageSet list);
  void RemoveViewCommand(IView* view, CRMessageSet list);

  void ExecuteCommand(const Message& message);
  BOOL HasCommand(const std::string& name) {
    return (m_commandMap.find(name) != m_commandMap.end());
  }

 private:
  MessageHandlerClass() = default;

  /* ICommand's deletion handled HERE, NOT refer to its CREATOR */
  std::unordered_map<std::string, ICommand*> m_commandMap;
  /* IView's deletion is handled by its CREATOR, NOT HERE */
  std::unordered_map<IView*, std::unordered_set<std::string>> m_viewCommandMap;
};

MessageHandlerClass& MessageHandler();
}  // namespace Naiive::Core
#endif
