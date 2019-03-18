#ifndef __CORE_ICOMMAND__
#define __CORE_ICOMMAND__
#include <stdafx.h>

#include <Core/Message.hpp>

namespace Naiive::Core {
class ICommand {
 public:
  ICommand() = default;
  virtual void operator()(Message msg) = 0;
  virtual ~ICommand() = default;
};
}  // namespace Naiive::Core
#endif
