#include "System.hpp"

namespace Naiive::Core {
SystemClass& System() {
  static SystemClass obj;
  return obj;
}
}  // namespace Naiive::Core
