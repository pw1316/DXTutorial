#ifndef __PW_ENTITY_MODEL3D__
#define __PW_ENTITY_MODEL3D__
#include <stdafx.h>
#include <string>
#include <vector>

#include <D3D11.h>
#include <D3DX10math.h>
#include <3rdparty/include/tiny_obj_loader.h>

#include <Core/Interface/IView.hpp>

namespace PW
{
    namespace Entity
    {
        class Model3D : public Core::IView
        {
        private:
            struct VBType
            {
                D3DXVECTOR3 pos;
                D3DXVECTOR2 uv;
                D3DXVECTOR3 normal;
            };
            struct CBTransformType
            {
                D3DXMATRIX world;
                D3DXMATRIX view;
                D3DXMATRIX proj;
            };
            struct CBCameraLightType
            {
                D3DXVECTOR4 camPos;
                D3DXVECTOR3 lightDir;
                float padding;
            };
            struct CBMaterialType
            {
                D3DXVECTOR4 ka;
                D3DXVECTOR4 kd;
                D3DXVECTOR4 ks;
                float ns;
                D3DXVECTOR3 padding;
            };
            struct TinyObj
            {
                tinyobj::attrib_t attr;
                std::vector<tinyobj::shape_t> shapes;
                std::vector<tinyobj::material_t> materials;
            };
        public:
            Model3D() = delete;
            explicit Model3D(const std::string path):m_name(path) {}
            ~Model3D() = default;

            /* Override */
            virtual HRESULT Awake() override { return S_OK; };
            virtual void Destroy() override {};
            virtual void OnMessage(const Core::Message &msg) override {};

            void Initialize(ID3D11Device *device);
            void Shutdown();
            void Render(ID3D11DeviceContext *context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, D3DXVECTOR4 camPos, D3DXVECTOR3 dir);
        private:
            /* Resources */
            void InitializeBuffer(ID3D11Device *device);
            void ShutdownBuffer();

            /* Shader */
            void InitializeShader(ID3D11Device *device);
            void ShutdownShader();

            std::string m_name;

            /* Resources */
            UINT m_VN = 0;
            ID3D11Buffer *m_VB = nullptr;
            ID3D11Buffer *m_IB = nullptr;
            ID3D11Buffer *m_CBTransform = nullptr;
            ID3D11Buffer *m_CBCameraLight = nullptr;
            ID3D11Buffer *m_CBMaterial = nullptr;
            ID3D11ShaderResourceView *m_SRVTexture0 = nullptr;
            ID3D11SamplerState *m_SamplerState = nullptr;

            /* Shader */
            ID3D11VertexShader *m_VS = nullptr;
            ID3D11PixelShader *m_PS = nullptr;
            ID3D11InputLayout *m_Layout = nullptr;
        };
    }
}
#endif

