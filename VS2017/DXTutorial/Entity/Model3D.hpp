#ifndef __ENTITY_MODEL3D__
#define __ENTITY_MODEL3D__
#include <stdafx.h>

#include <string>
#include <vector>

#include <3rdparty/include/tiny_obj_loader.h>
#include <DirectXMath.h>
#include <d3d11.h>

#include <Core/Interface/IView.hpp>

namespace PW {
namespace Entity {
class Model3D : public Naiive::Core::IView {
 private:
  struct VBType {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT3 normal;
  };
  struct CBTransformType {
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 proj;
  };
  struct CBCameraLightType {
    DirectX::XMFLOAT4 camPos;
    DirectX::XMFLOAT3 lightDir;
    float padding;
  };
  struct CBMaterialType {
    DirectX::XMFLOAT4 ka;
    DirectX::XMFLOAT4 kd;
    DirectX::XMFLOAT4 ks;
    float ns;
    DirectX::XMFLOAT3 padding;
  };
  struct TinyObj {
    tinyobj::attrib_t attr;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
  };

 public:
  Model3D() = delete;
  explicit Model3D(const std::string path) : m_name(path) {}
  ~Model3D() = default;

  /* Override */
  virtual void Awake() override{};
  virtual void Destroy() override{};
  virtual void OnMessage(const Naiive::Core::Message& msg) override{};

  void Initialize(ID3D11Device* device);
  void Shutdown();
  void Render(ID3D11DeviceContext* context, DirectX::XMFLOAT4X4 view,
              DirectX::XMFLOAT4X4 proj, DirectX::XMFLOAT4 camPos,
              DirectX::XMFLOAT3 dir);

 private:
  /* Resources */
  void InitializeBuffer(ID3D11Device* device);
  void ShutdownBuffer();

  /* Shader */
  void InitializeShader(ID3D11Device* device);
  void ShutdownShader();

  std::string m_name;
  float rotation = 0.0f;

  /* Resources */
  UINT m_VN = 0;
  ID3D11Buffer* m_VB = nullptr;
  ID3D11Buffer* m_IB = nullptr;
  ID3D11Buffer* m_CBTransform = nullptr;
  ID3D11Buffer* m_CBCameraLight = nullptr;
  ID3D11Buffer* m_CBMaterial = nullptr;
  ID3D11ShaderResourceView* m_SRVTexture0 = nullptr;
  ID3D11SamplerState* m_SamplerState = nullptr;

  /* Shader */
  ID3D11VertexShader* m_VS = nullptr;
  ID3D11PixelShader* m_PS = nullptr;
  ID3D11InputLayout* m_Layout = nullptr;
};
}  // namespace Entity
}  // namespace PW
#endif
