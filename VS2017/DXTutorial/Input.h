#pragma once
#include "stdafx.h"

class Input
{
public:
    Input() {}
    Input(const Input& rhs) {}
    Input(Input&& rhs) {}
    ~Input() {}

    void Initialize();

    void KeyDown(unsigned int);
    void KeyUp(unsigned int);

    bool IsKeyDown(unsigned int);

private:
    bool m_keys[256];
};
