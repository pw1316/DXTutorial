#pragma once
#include "stdafx.h"

#include <D3D11.h>
#include <D3DX10math.h>
#include <d3dx11async.h>

class Shader
{
private:
    struct DXMatrix
    {
        D3DXMATRIX world;
        D3DXMATRIX view;
        D3DXMATRIX proj;
    };

    struct DXLight
    {
        D3DXVECTOR4 ambient;
        D3DXVECTOR4 diffuse;
        D3DXVECTOR3 dir;
        float padding;// 16x bytes
    };
public:
    Shader() = default;
    Shader(const Shader &rhs) = default;
    Shader(Shader &&rhs) = default;
    ~Shader() = default;

    HRESULT Initialize(ID3D11Device *device, HWND hwnd);
    void Shutdown();
    HRESULT Render(ID3D11DeviceContext *context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, ID3D11ShaderResourceView *texture, D3DXVECTOR4 diffuse, D3DXVECTOR3 dir);
private:
    HRESULT InitializeShader(ID3D11Device *device, HWND hwnd);
    void ShutdownShader();
    HRESULT RenderShader(ID3D11DeviceContext *context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj);

    ID3D11VertexShader *m_VS = nullptr;
    ID3D11PixelShader *m_PS = nullptr;
    ID3D11InputLayout *m_layout = nullptr;
    ID3D11Buffer *m_matrix = nullptr;
    ID3D11SamplerState *m_samplerState = nullptr;
    ID3D11Buffer *m_light = nullptr;
};
