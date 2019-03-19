#include <stdafx.h>

#include "InputManager.hpp"

namespace Naiive::Manager {
InputManagerClass& InputManager() {
  static InputManagerClass obj;
  return obj;
}
}  // namespace Naiive::Manager
