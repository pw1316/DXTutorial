#pragma once
#include "stdafx.h"

class Input
{
private:
    static const UINT NUM_KEYS = 256U;
public:
    Input() = default;
    Input(const Input& rhs) = default;
    Input(Input&& rhs) {}
    ~Input() {}

    void Initialize();

    void KeyDown(std::remove_const<decltype(NUM_KEYS)>::type);
    void KeyUp(std::remove_const<decltype(NUM_KEYS)>::type);

    BOOL IsKeyDown(std::remove_const<decltype(NUM_KEYS)>::type);
private:
    BOOL m_keys[NUM_KEYS];
};
