#include "stdafx.h"
#include "Shader.h"

HRESULT Shader::Initialize(ID3D11Device *device, HWND hwnd)
{
    return InitializeShader(device, hwnd);
}

void Shader::Shutdown()
{
    ShutdownShader();
}

HRESULT Shader::Render(ID3D11DeviceContext *context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, ID3D11ShaderResourceView *texture)
{
    HRESULT hr = S_OK;
    D3D11_MAPPED_SUBRESOURCE mapped{};
    DXMatrix *data = nullptr;
    UINT nBuffer = 0;

    D3DXMatrixTranspose(&world, &world);
    D3DXMatrixTranspose(&view, &view);
    D3DXMatrixTranspose(&proj, &proj);
    hr = context->Map(m_matrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        return E_FAIL;
    }
    data = (DXMatrix *)mapped.pData;
    data->world = world;
    data->view = view;
    data->proj = proj;
    context->Unmap(m_matrix, 0);
    nBuffer = 0;
    context->VSSetConstantBuffers(nBuffer, 1, &m_matrix);
    context->PSSetShaderResources(0, 1, &texture);
    return RenderShader(context, indexCount, world, view, proj);
}

HRESULT Shader::InitializeShader(ID3D11Device *device, HWND hwnd)
{
    HRESULT hrVS = S_OK, hrPS = S_OK;
    ID3D10Blob *VSBuffer = nullptr;
    ID3D10Blob *PSBuffer = nullptr;
    D3D11_INPUT_ELEMENT_DESC layout[2] = {};
    UINT nLayout = 0;
    D3D11_BUFFER_DESC matrixDesc{};
    D3D11_SAMPLER_DESC sampleDesc{};

    hrVS = D3DX11CompileFromFile(L"VS.hlsl", nullptr, nullptr, "VS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &VSBuffer, nullptr, nullptr);
    hrPS = D3DX11CompileFromFile(L"PS.hlsl", nullptr, nullptr, "PS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &PSBuffer, nullptr, nullptr);
    if (FAILED(hrVS) || FAILED(hrPS))
    {
        SafeRelease(&VSBuffer);
        SafeRelease(&PSBuffer);
        return E_FAIL;
    }
    hrVS = device->CreateVertexShader(VSBuffer->GetBufferPointer(), VSBuffer->GetBufferSize(), nullptr, &m_VS);
    hrPS = device->CreatePixelShader(PSBuffer->GetBufferPointer(), PSBuffer->GetBufferSize(), nullptr, &m_PS); 
    if (FAILED(hrVS) || FAILED(hrPS))
    {
        SafeRelease(&VSBuffer);
        SafeRelease(&PSBuffer);
        SafeRelease(&m_VS);
        SafeRelease(&m_PS);
        return E_FAIL;
    }

    layout[0].SemanticName = "POSITION";
    layout[0].SemanticIndex = 0;
    layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    layout[0].InputSlot = 0;
    layout[0].AlignedByteOffset = 0;
    layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[0].InstanceDataStepRate = 0;
    layout[1].SemanticName = "TEXCOORD";
    layout[1].SemanticIndex = 0;
    layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    layout[1].InputSlot = 0;
    layout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[1].InstanceDataStepRate = 0;
    nLayout = sizeof(layout) / sizeof(layout[0]);
    hrVS = device->CreateInputLayout(layout, nLayout, VSBuffer->GetBufferPointer(), VSBuffer->GetBufferSize(), &m_layout);
    SafeRelease(&VSBuffer);
    SafeRelease(&PSBuffer);
    if (FAILED(hrVS))
    {
        SafeRelease(&m_PS);
        SafeRelease(&m_VS);
        return E_FAIL;
    }

    matrixDesc.ByteWidth = sizeof(DXMatrix);
    matrixDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixDesc.MiscFlags = 0;
    matrixDesc.StructureByteStride = 0;
    hrVS = device->CreateBuffer(&matrixDesc, nullptr, &m_matrix);
    if (FAILED(hrVS))
    {
        SafeRelease(&m_layout);
        SafeRelease(&m_PS);
        SafeRelease(&m_VS);
        return E_FAIL;
    }

    sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampleDesc.MipLODBias = 0.0f;
    sampleDesc.MaxAnisotropy = 1;
    sampleDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampleDesc.BorderColor[0] = 0;
    sampleDesc.BorderColor[1] = 0;
    sampleDesc.BorderColor[2] = 0;
    sampleDesc.BorderColor[3] = 0;
    sampleDesc.MinLOD = 0;
    sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hrVS = device->CreateSamplerState(&sampleDesc, &m_samplerState);
    if (FAILED(hrVS))
    {
        SafeRelease(&m_matrix);
        SafeRelease(&m_layout);
        SafeRelease(&m_PS);
        SafeRelease(&m_VS);
        return E_FAIL;
    }
    return S_OK;
}

void Shader::ShutdownShader()
{
    SafeRelease(&m_samplerState);
    SafeRelease(&m_matrix);
    SafeRelease(&m_layout);
    SafeRelease(&m_PS);
    SafeRelease(&m_VS);
}

HRESULT Shader::RenderShader(ID3D11DeviceContext *context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
    context->IASetInputLayout(m_layout);
    context->VSSetShader(m_VS, nullptr, 0);
    context->PSSetShader(m_PS, nullptr, 0);
    context->PSSetSamplers(0, 1, &m_samplerState);
    context->DrawIndexed(indexCount, 0, 0);
    return S_OK;
}
