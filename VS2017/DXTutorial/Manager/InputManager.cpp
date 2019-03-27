#include "InputManager.hpp"

namespace Naiive::Manager {
InputManagerClass& InputManager() {
  static InputManagerClass obj;
  return obj;
}

void InputManagerClass::Initialize(HWND hWnd, UINT width, UINT height) {
  HRESULT hr = S_OK;
  auto hInst = HINSTANCE_FROM_HWND(hWnd);

  m_w = width;
  m_h = height;
  hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8,
                          (void**)&m_dinput, nullptr);
  FAILTHROW;
  hr = m_dinput->CreateDevice(GUID_SysKeyboard, &m_keyboard, nullptr);
  FAILTHROW;
  hr = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
  FAILTHROW;
  hr =
      m_keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
  FAILTHROW;
  hr = m_keyboard->Acquire();
  FAILTHROW;

  hr = m_dinput->CreateDevice(GUID_SysMouse, &m_mouse, nullptr);
  FAILTHROW;
  hr = m_mouse->SetDataFormat(&c_dfDIMouse);
  FAILTHROW;
  hr =
      m_mouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
  FAILTHROW;
  hr = m_mouse->Acquire();
  FAILTHROW;
}

void InputManagerClass::Shutdown() {
  m_mouse->Unacquire();
  SafeRelease(&m_mouse);
  m_keyboard->Unacquire();
  SafeRelease(&m_keyboard);
  SafeRelease(&m_dinput);
}

BOOL InputManagerClass::OnUpdate() {
  HRESULT hr = S_OK;
  hr = m_keyboard->GetDeviceState(sizeof(m_keyState), &m_keyState[0]);
  if (FAILED(hr)) {
    if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
      m_keyboard->Acquire();
    } else {
      return FALSE;
    }
  }
  hr = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
  if (FAILED(hr)) {
    if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
      m_mouse->Acquire();
    } else {
      return FALSE;
    }
  }

  m_x += m_mouseState.lX;
  m_y += m_mouseState.lY;

  // if (m_x < 0) {
  //  m_x = 0;
  //}
  // if (m_y < 0) {
  //  m_y = 0;
  //}

  // if (m_x > m_w) {
  //  m_x = m_w;
  //}
  // if (m_y > m_h) {
  //  m_y = m_h;
  //}
  return TRUE;
}
}  // namespace Naiive::Manager
