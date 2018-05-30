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

HRESULT Shader::Render(ID3D11DeviceContext *context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, D3DXVECTOR3 camPos, ID3D11ShaderResourceView *texture, D3DXVECTOR4 diffuse, D3DXVECTOR4 specular, D3DXVECTOR3 dir)
{
    HRESULT hr = S_OK;
    D3D11_MAPPED_SUBRESOURCE mapped{};
    DXMatrix *dataM = nullptr;
    DXCamera *dataC = nullptr;
    DXLight *dataL = nullptr;
    UINT nBuffer = 0;

    D3DXMatrixTranspose(&world, &world);
    D3DXMatrixTranspose(&view, &view);
    D3DXMatrixTranspose(&proj, &proj);
    hr = context->Map(m_matrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        return E_FAIL;
    }
    dataM = (DXMatrix *)mapped.pData;
    dataM->world = world;
    dataM->view = view;
    dataM->proj = proj;
    context->Unmap(m_matrix, 0);
    nBuffer = 0;
    context->VSSetConstantBuffers(nBuffer, 1, &m_matrix);

    hr = context->Map(m_camera, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        return E_FAIL;
    }
    dataC = (DXCamera *)mapped.pData;
    dataC->pos = camPos;
    context->Unmap(m_camera, 0);
    nBuffer = 1;
    context->VSSetConstantBuffers(nBuffer, 1, &m_camera);
    
    context->PSSetShaderResources(0, 1, &texture);

    hr = context->Map(m_light, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        return E_FAIL;
    }
    dataL = (DXLight*)mapped.pData;
    dataL->ambient = D3DXVECTOR4(0.05f, 0.05f, 0.05f, 1.0f);
    dataL->diffuse = diffuse;
    dataL->specular = specular;
    dataL->dir = dir;
    dataL->ns = 10.0f;
    context->Unmap(m_light, 0);
    nBuffer = 0;
    context->PSSetConstantBuffers(nBuffer, 1, &m_light);

    return RenderShader(context, indexCount, world, view, proj);
}

HRESULT Shader::InitializeShader(ID3D11Device *device, HWND hwnd)
{
    HRESULT hrVS = S_OK, hrPS = S_OK;
    ID3D10Blob *VSBuffer = nullptr;
    ID3D10Blob *PSBuffer = nullptr;
    D3D11_INPUT_ELEMENT_DESC layout[3] = {};
    UINT nLayout = 0;
    D3D11_BUFFER_DESC matrixDesc{};
    D3D11_BUFFER_DESC cameraDesc{};
    D3D11_SAMPLER_DESC sampleDesc{};
    D3D11_BUFFER_DESC lightDesc{};

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
    layout[2].SemanticName = "NORMAL";
    layout[2].SemanticIndex = 0;
    layout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    layout[2].InputSlot = 0;
    layout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    layout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[2].InstanceDataStepRate = 0;
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

    cameraDesc.ByteWidth = sizeof(DXCamera);
    cameraDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraDesc.MiscFlags = 0;
    cameraDesc.StructureByteStride = 0;
    hrVS = device->CreateBuffer(&cameraDesc, nullptr, &m_camera);
    if (FAILED(hrVS))
    {
        SafeRelease(&m_matrix);
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
        SafeRelease(&m_camera);
        SafeRelease(&m_matrix);
        SafeRelease(&m_layout);
        SafeRelease(&m_PS);
        SafeRelease(&m_VS);
        return E_FAIL;
    }

    lightDesc.ByteWidth = sizeof(DXLight);
    lightDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightDesc.MiscFlags = 0;
    lightDesc.StructureByteStride = 0;
    hrVS = device->CreateBuffer(&lightDesc, nullptr, &m_light);
    if (FAILED(hrVS))
    {
        SafeRelease(&m_samplerState);
        SafeRelease(&m_camera);
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
    SafeRelease(&m_light);
    SafeRelease(&m_samplerState);
    SafeRelease(&m_camera);
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
