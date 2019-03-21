#ifndef __CORE_ICOMMAND__
#define __CORE_ICOMMAND__
#include <stdafx.h>

namespace Naiive::Core {
class Message;

class ICommand {
 public:
  ICommand() = default;
  virtual ~ICommand() = default;
  virtual void operator()(const Message& msg) = 0;
  virtual void operator()(Message&& msg) = 0;
};
}  // namespace Naiive::Core
#endif
