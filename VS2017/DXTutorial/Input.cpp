#include "stdafx.h"

#include "Input.h"

void Input::Initialize()
{
    for (std::remove_const<decltype(NUM_KEYS)>::type i = 0; i < NUM_KEYS; ++i)
    {
        m_keys[i] = false;
    }
}

void Input::KeyDown(std::remove_const<decltype(NUM_KEYS)>::type key)
{
    assert(key < NUM_KEYS);
    m_keys[key] = true;
}

void Input::KeyUp(std::remove_const<decltype(NUM_KEYS)>::type key)
{
    assert(key < NUM_KEYS);
    m_keys[key] = false;
}

BOOL Input::IsKeyDown(std::remove_const<decltype(NUM_KEYS)>::type key)
{
    assert(key < 256);
    return m_keys[key];
}
