#ifndef __MANAGER_INPUT_MANAGER__
#define __MANAGER_INPUT_MANAGER__
#include <stdafx.h>

#include <algorithm>

#include <dinput.h>

#include <Core/Interface/IView.hpp>

namespace Naiive::Manager {
class InputManagerClass : public Core::IView {
  friend InputManagerClass& InputManager();

 private:
  static constexpr UINT NUM_KEYS = 256U;

 public:
  /* Override */
  virtual void Initialize(HWND hWnd, UINT width, UINT height) override;
  virtual void Shutdown() override;
  virtual BOOL OnUpdate() override;

  BOOL IsKeyDown(std::remove_const<decltype(NUM_KEYS)>::type key) {
    assert(key < 256);
    return m_keyState[key] & 0x80;
  }

  void GetMouse(LONG& x, LONG& y) {
    x = std::clamp(m_x, 0L, m_w);
    y = std::clamp(m_y, 0L, m_h);
  }

 private:
  InputManagerClass() {}
  virtual ~InputManagerClass() {}

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
