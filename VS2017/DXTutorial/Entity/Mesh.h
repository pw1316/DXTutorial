#pragma once
#include "stdafx.h"

#include <D3D11.h>
#include <D3DX10math.h>

#include "3rdparty/include/tiny_obj_loader.h"
#include "Texture.h"

#include <string>

class Mesh
{
private:
    struct DXVertex
    {
        D3DXVECTOR3 pos;
        D3DXVECTOR2 uv;
        D3DXVECTOR3 normal;
    };
    struct TinyObj
    {
        tinyobj::attrib_t attr;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
    };
public:
    Mesh() = delete;
    Mesh(const std::wstring name) :m_name(name) {}
    Mesh(const Mesh &rhs) = default;
    Mesh(Mesh &&rhs) = default;
    ~Mesh() = default;

    HRESULT Initialize(ID3D11Device *device);
    void Shutdown();
    void Render(ID3D11DeviceContext *context);
    int GetIndexCount();
    ID3D11ShaderResourceView *GetTexture();
private:
    HRESULT InitializeBuffer(ID3D11Device *device);
    HRESULT InitializeModel(ID3D11Device *device);
    HRESULT InitializeTexture(ID3D11Device *device);
    void ShutdownBuffer();
    void ShutdownModel();
    void ShutdownTexture();
    void RenderBuffer(ID3D11DeviceContext *context);

    const std::wstring m_name;
    ID3D11Buffer *m_VB = nullptr;
    ID3D11Buffer *m_IB = nullptr;
    UINT m_VBN = 0;
    UINT m_IBN = 0;
    TinyObj *m_model = nullptr;
    Texture *m_texture = nullptr;
};
