#pragma once
#include "stdafx.h"

class Graphics
{
public:
    Graphics() {}
    Graphics(const Graphics& rhs) {}
    Graphics(Graphics&& rhs) {}
    ~Graphics() {}

    HRESULT Initialize();
    HRESULT Shutdown();
    HRESULT OnRender();
};