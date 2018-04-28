#include "stdafx.h"
#include "Texture.h"

HRESULT Texture::Initialize(ID3D11Device *device, LPCWSTR fileName)
{
    SafeRelease(&m_texture);
    HRESULT hr = S_OK;
    hr = D3DX11CreateShaderResourceViewFromFile(device, fileName, nullptr, nullptr, &m_texture, nullptr);
    return hr;
}

void Texture::Shutdown()
{
    SafeRelease(&m_texture);
}

ID3D11ShaderResourceView * Texture::GetTexture()
{
    return m_texture;
}
