#include <stdafx.h>

#include "MessageHandler.hpp"

namespace Naiive::Core {
MessageHandlerClass& Naiive::Core::MessageHandler() {
  static MessageHandlerClass obj;
  return obj;
}
}  // namespace Naiive::Core
