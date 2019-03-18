#ifndef __MANAGER_INPUT_MANAGER__
#define __MANAGER_INPUT_MANAGER__
#include <stdafx.h>

#include <Core/Interface/IView.hpp>
#include <Core/MessageHandler.hpp>
#include <Core/System.hpp>

namespace Naiive::Manager {
class InputManager : public Core::IView {
 private:
  static const UINT NUM_KEYS = 256U;

 public:
  InputManager() = default;
  ~InputManager() {}

  /* Override */
  virtual HRESULT Awake() override {
    messages = {"SYS_KEY_DOWN", "SYS_KEY_UP"};
    Core::MessageHandler().RegisterViewCommand(this, messages);
    return S_OK;
  }
  virtual void Destroy() override {
    Core::MessageHandler().RemoveViewCommand(this, messages);
  }
  virtual void OnMessage(const Core::Message& msg) override {
    if (msg.GetName() == "SYS_KEY_DOWN") {
      KeyDown(msg.GetBody());
    } else if (msg.GetName() == "SYS_KEY_UP") {
      KeyUp(msg.GetBody());
    }
  }

  BOOL IsKeyDown(std::remove_const<decltype(NUM_KEYS)>::type key) {
    assert(key < 256);
    return m_keys[key];
  }

 private:
  void KeyDown(std::remove_const<decltype(NUM_KEYS)>::type key) {
    assert(key < NUM_KEYS);
    Core::System().DebugInfo(key, " down");
    m_keys[key] = true;
  }
  void KeyUp(std::remove_const<decltype(NUM_KEYS)>::type key) {
    assert(key < NUM_KEYS);
    Core::System().DebugInfo(key, " up");
    m_keys[key] = false;
  }
  BOOL m_keys[NUM_KEYS];
};
}  // namespace Naiive::Manager

#endif
