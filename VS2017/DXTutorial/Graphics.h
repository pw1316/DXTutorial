#pragma once
#include "stdafx.h"

#include <DXGI.h>
#include <D3D11.h>

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
private:
    HRESULT BeginScene();
    HRESULT EndScene();
};