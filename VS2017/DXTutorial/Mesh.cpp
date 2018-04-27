#include "stdafx.h"
#include "Mesh.h"

HRESULT Mesh::Initialize(ID3D11Device *device)
{
    return InitializeBuffer(device);
}

void Mesh::Shutdown()
{
    ShutdownBuffer();
}

void Mesh::Render(ID3D11DeviceContext *context)
{
    RenderBuffer(context);
}

int Mesh::GetIndexCount()
{
    return m_IBN;
}

HRESULT Mesh::InitializeBuffer(ID3D11Device * device)
{
    HRESULT hrv = S_OK, hri = S_OK;
    DXVertex *vertices = nullptr;
    ULONG *indices = nullptr;
    D3D11_BUFFER_DESC VBDesc{}, IBDesc{};
    D3D11_SUBRESOURCE_DATA VData{}, IData{};

    /* Triangle */
    m_VBN = 3U;
    m_IBN = 3U;

    vertices = new DXVertex[m_VBN];
    indices = new ULONG[m_IBN];
    vertices[0].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);//BL
    vertices[0].color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
    vertices[1].pos = D3DXVECTOR3(0.0f, 1.0f, 0.0f);//T
    vertices[1].color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
    vertices[2].pos = D3DXVECTOR3(1.0f, -1.0f, 0.0f);//BR
    vertices[2].color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
    indices[0] = 0;//BL
    indices[1] = 1;//T
    indices[2] = 2;//BR

    VBDesc.ByteWidth = sizeof(DXVertex) * m_VBN;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = 0;
    VData.pSysMem = vertices;
    VData.SysMemPitch = 0;
    VData.SysMemSlicePitch = 0;
    hrv = device->CreateBuffer(&VBDesc, &VData, &m_VB);

    IBDesc.ByteWidth = sizeof(ULONG) * m_VBN;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags = 0;
    IBDesc.MiscFlags = 0;
    IBDesc.StructureByteStride = 0;
    IData.pSysMem = indices;
    IData.SysMemPitch = 0;
    IData.SysMemSlicePitch = 0;
    hri = device->CreateBuffer(&IBDesc, &IData, &m_IB);
    delete[] vertices;
    vertices = nullptr;
    delete[] indices;
    indices = nullptr;
    if (FAILED(hrv) || FAILED(hri))
    {
        SafeRelease(&m_VB);
        SafeRelease(&m_IB);
        m_VBN = 0;
        m_IBN = 0;
        return E_FAIL;
    }
    return S_OK;
}

void Mesh::ShutdownBuffer()
{
    SafeRelease(&m_VB);
    SafeRelease(&m_IB);
    m_VBN = 0;
    m_IBN = 0;
}

void Mesh::RenderBuffer(ID3D11DeviceContext *context)
{
    UINT stride = sizeof(DXVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
    context->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
