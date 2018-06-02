#include "stdafx.h"
#include "Mesh.h"

HRESULT Mesh::Initialize(ID3D11Device *device)
{
    HRESULT hr = S_OK;
    hr = InitializeModel(device);
    FAILRETURN();
    hr = InitializeBuffer(device);
    if (FAILED(hr))
    {
        ShutdownModel();
        return E_FAIL;
    }
    hr = InitializeTexture(device);
    if (FAILED(hr))
    {
        ShutdownBuffer();
        ShutdownModel();
        return E_FAIL;
    }
    return S_OK;
}

void Mesh::Shutdown()
{
    ShutdownTexture();
    ShutdownBuffer();
    ShutdownModel();
}

void Mesh::Render(ID3D11DeviceContext *context)
{
    RenderBuffer(context);
}

int Mesh::GetIndexCount()
{
    return m_IBN;
}

ID3D11ShaderResourceView *Mesh::GetTexture()
{
    assert(m_texture != nullptr);
    return m_texture->GetTexture();
}

HRESULT Mesh::InitializeBuffer(ID3D11Device *device)
{
    HRESULT hrv = S_OK, hri = S_OK;
    DXVertex *vertices = nullptr;
    ULONG *indices = nullptr;
    D3D11_BUFFER_DESC VBDesc{}, IBDesc{};
    D3D11_SUBRESOURCE_DATA VData{}, IData{};

    /* Triangle */
    m_VBN = m_model->shapes[0].mesh.indices.size();
    m_IBN = m_model->shapes[0].mesh.indices.size();

    vertices = new DXVertex[m_VBN];
    indices = new ULONG[m_IBN];
    for (UINT i = 0; i < m_VBN / 3; ++i)
    {
        for (UINT j = 0; j < 3; ++j)
        {
            auto vi = m_model->shapes[0].mesh.indices[3 * i + 2 - j].vertex_index;
            auto ti = m_model->shapes[0].mesh.indices[3 * i + 2 - j].texcoord_index;
            auto ni = m_model->shapes[0].mesh.indices[3 * i + 2 - j].normal_index;
            auto x = m_model->attr.vertices[3 * vi + 0];
            auto y = m_model->attr.vertices[3 * vi + 1];
            auto z = m_model->attr.vertices[3 * vi + 2];
            vertices[3 * i + j].pos = D3DXVECTOR3(x, y, -z);
            x = m_model->attr.texcoords[2 * ti + 0];
            y = m_model->attr.texcoords[2 * ti + 1];
            vertices[3 * i + j].uv = D3DXVECTOR2(x, 1.0 - y);
            x = m_model->attr.normals[3 * ni + 0];
            y = m_model->attr.normals[3 * ni + 1];
            z = m_model->attr.normals[3 * ni + 2];
            vertices[3 * i + j].normal = D3DXVECTOR3(x, y, -z);
            indices[3 * i + j] = 3 * i + j;
        }
    }

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

HRESULT Mesh::InitializeModel(ID3D11Device * device)
{
    if (m_model == nullptr)
    {
        m_model = new TinyObj;
    }
    std::string charname;
    for (auto &each : m_name + L".obj")
    {
        charname.push_back(each & 0x00FF);
    }
    bool res = tinyobj::LoadObj(&m_model->attr, &m_model->shapes, &m_model->materials, nullptr, charname.c_str(), nullptr, true);
    if (!res)
    {
        return E_FAIL;
    }
    return S_OK;
}

HRESULT Mesh::InitializeTexture(ID3D11Device *device)
{
    if (m_texture == nullptr)
    {
        m_texture = new Texture;
    }
    return m_texture->Initialize(device, (m_name + L".dds").c_str());;
}

void Mesh::ShutdownBuffer()
{
    SafeRelease(&m_VB);
    SafeRelease(&m_IB);
    m_VBN = 0;
    m_IBN = 0;
}

void Mesh::ShutdownModel()
{
    if (m_model != nullptr)
    {
        delete m_model;
        m_model = nullptr;
    }
}

void Mesh::ShutdownTexture()
{
    if (m_texture != nullptr)
    {
        m_texture->Shutdown();
        delete m_texture;
        m_texture = nullptr;
    }
}

void Mesh::RenderBuffer(ID3D11DeviceContext *context)
{
    UINT stride = sizeof(DXVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
    context->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
