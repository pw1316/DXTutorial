#pragma once
#include "stdafx.h"

#include <D3D11.h>
#include <D3DX11tex.h>

class Texture
{
public:
    Texture() = default;
    Texture(const Texture &rhs) = default;
    Texture(Texture &&rhs) = default;
    ~Texture() = default;

    HRESULT Initialize(ID3D11Device *device, LPCWSTR fileName);
    void Shutdown();
    ID3D11ShaderResourceView *GetTexture();
private:
    ID3D11ShaderResourceView * m_texture = nullptr;
};
