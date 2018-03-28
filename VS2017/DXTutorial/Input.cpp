#include "stdafx.h"
#include "Input.h"
#include <assert.h>

void Input::Initialize()
{
    for (int i = 0; i < 256; i++)
    {
        m_keys[i] = false;
    }
}

void Input::KeyDown(unsigned int key)
{
    assert(key < 256);
    m_keys[key] = true;
}

void Input::KeyUp(unsigned int key)
{
    assert(key < 256);
    m_keys[key] = false;
}

bool Input::IsKeyDown(unsigned int key)
{
    assert(key < 256);
    return m_keys[key];
}
