#ifndef __ENTITY_FONT__
#define __ENTITY_FONT__
#include <stdafx.h>

#include <string>

#include <DirectXMath.h>
#include <d3d11_4.h>

#include <Core/Interface/IView.hpp>

namespace PW::Entity {
class Font : public Naiive::Core::IView {
 private:
  struct FontType {
    float left, right;
    int size;
  };
  struct VBType {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
  };
  struct CBTransformType {
    DirectX::XMFLOAT4X4 proj;
  };
  struct CBColorType {
    DirectX::XMFLOAT4 color;
  };

 public:
  Font() = default;
  ~Font() = default;

  /* Override */
  virtual void Awake() override{};
  virtual void Destroy() override{};
  virtual void OnMessage(const Naiive::Core::Message& msg) override{};

  void Initialize(ID3D11Device* device);
  void Shutdown();
  void Render(ID3D11Device* device, ID3D11DeviceContext* context,
              const std::string& text, const DirectX::XMFLOAT2& pos,
              DirectX::XMFLOAT4X4 proj);

 private:
  /* Resources */
  void InitializeBuffer(ID3D11Device* device);
  void ShutdownBuffer();

  /* Shader */
  void InitializeShader(ID3D11Device* device);
  void ShutdownShader();

  FontType m_Font[95];

  /* Resources */
  ID3D11Buffer* m_VB = nullptr;
  ID3D11Buffer* m_IB = nullptr;
  ID3D11Buffer* m_CBTransform = nullptr;
  ID3D11Buffer* m_CBColor = nullptr;
  ID3D11ShaderResourceView* m_SRVTexture = nullptr;
  ID3D11SamplerState* m_SamplerState = nullptr;

  /* Shader */
  ID3D11VertexShader* m_VS = nullptr;
  ID3D11PixelShader* m_PS = nullptr;
  ID3D11InputLayout* m_Layout = nullptr;
};
}  // namespace PW::Entity

#endif
