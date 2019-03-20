#ifndef __MANAGER_INPUT_MANAGER__
#define __MANAGER_INPUT_MANAGER__
#include <stdafx.h>

#include <dinput.h>

#include <Core/Interface/IView.hpp>
#include <Core/MessageHandler.hpp>
#include <Core/System.hpp>

namespace Naiive::Manager {
class InputManagerClass : public Core::IView {
  friend InputManagerClass& InputManager();

 private:
  static constexpr UINT NUM_KEYS = 256U;

 public:
  /* Override */
  virtual void Awake() override {
    // messages = {"SYS_KEY_DOWN", "SYS_KEY_UP"};
    // Core::MessageHandler().RegisterViewCommand(this, messages);
  }
  virtual void Destroy() override {
    // Core::MessageHandler().RemoveViewCommand(this, messages);
  }
  virtual void OnMessage(const Core::Message& msg) override {
    // if (msg.GetName() == "SYS_KEY_DOWN") {
    //  KeyDown(msg.GetBody());
    //} else if (msg.GetName() == "SYS_KEY_UP") {
    //  KeyUp(msg.GetBody());
    //}
  }

  void Initialize(HWND hWnd, UINT width, UINT height);
  void Shutdown();
  BOOL OnUpdate();

  BOOL IsKeyDown(std::remove_const<decltype(NUM_KEYS)>::type key) {
    assert(key < 256);
    return m_keyState[key] & 0x80;
  }

  void GetMouse(LONG& x, LONG& y) {
    x = m_x;
    y = m_y;
  }

 private:
  InputManagerClass() = default;

  IDirectInput8* m_dinput = nullptr;
  IDirectInputDevice8* m_keyboard = nullptr;
  IDirectInputDevice8* m_mouse = nullptr;
  UCHAR m_keyState[NUM_KEYS];
  DIMOUSESTATE m_mouseState;
  LONG m_x = 0, m_y = 0;
  LONG m_w = 0, m_h = 0;
};

InputManagerClass& InputManager();
}  // namespace Naiive::Manager

#endif
