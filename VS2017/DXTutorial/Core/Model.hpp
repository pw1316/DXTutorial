#ifndef __PW_CORE_MODEL__
#define __PW_CORE_MODEL__
#include <stdafx.h>
#include <string>
#include <vector>

#include <D3D11.h>
#include <D3DX10math.h>
#include <3rdparty/include/tiny_obj_loader.h>

#include <Core/Interface/IView.hpp>
#include <Shader.h>

namespace PW
{
    namespace Core
    {
        class Model : public IView
        {
        private:
            struct VertexType
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
            Model() = delete;
            explicit Model(const std::wstring path):m_name(path) {}
            ~Model() = default;

            /* Override */
            virtual HRESULT Awake() override { return S_OK; };
            virtual void Destroy() override {};
            virtual void OnMessage(const Message &msg) override {};

            HRESULT Initialize(ID3D11Device *device);
            void Shutdown();
            void Render(ID3D11DeviceContext *context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, D3DXVECTOR3 camPos, D3DXVECTOR3 dir);
        private:
            /* Mesh, Material, Texture */
            HRESULT LoadMesh(ID3D11Device *device);
            void UnLoadMesh();

            /* VertexBuffer */
            HRESULT InitializeBuffer(ID3D11Device *device);
            void ShutdownBuffer();

            std::wstring m_name;

            /* Mesh, Material, Texture */
            TinyObj *m_model = nullptr;
            ID3D11ShaderResourceView *m_texture = nullptr;

            /* VertexBuffer */
            ID3D11Buffer *m_VB = nullptr;
            ID3D11Buffer *m_IB = nullptr;
            UINT m_VBN = 0;
            UINT m_IBN = 0;

            /* Shader */
            Shader *m_shader = nullptr;
        };
    }
}
#endif

