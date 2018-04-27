#pragma once
#include "stdafx.h"

#include <D3D11.h>
#include <D3DX10math.h>

class Mesh
{
private:
    struct DXVertex
    {
        D3DXVECTOR3 pos;
        D3DXVECTOR4 color;
    };
public:
    Mesh() = default;
    Mesh(const Mesh &rhs) = default;
    Mesh(Mesh &&rhs) = default;
    ~Mesh() = default;

    HRESULT Initialize(ID3D11Device *device);
    void Shutdown();
    void Render(ID3D11DeviceContext *context);
    int GetIndexCount();
private:
    HRESULT InitializeBuffer(ID3D11Device *device);
    void ShutdownBuffer();
    void RenderBuffer(ID3D11DeviceContext *context);

    ID3D11Buffer *m_VB = nullptr;
    ID3D11Buffer *m_IB = nullptr;
    UINT m_VBN = 0;
    UINT m_IBN = 0;
};