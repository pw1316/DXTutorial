#ifndef __ENTITY_FONT__
#define __ENTITY_FONT__
#include <stdafx.h>

#include <string>

#include <D3D11.h>
#include <D3DX10math.h>

#include <Core/Interface/IView.hpp>

namespace PW {
namespace Entity {
class Font : public Naiive::Core::IView {
 private:
  struct FontType {
    float left, right;
    int size;
  };
  struct VBType {
    D3DXVECTOR3 pos;
    D3DXVECTOR2 uv;
  };
  struct CBTransformType {
    D3DXMATRIX proj;
  };
  struct CBColorType {
    D3DXVECTOR4 color;
  };

 public:
  Font() = default;
  ~Font() = default;

  /* Override */
  virtual HRESULT Awake() override { return S_OK; };
  virtual void Destroy() override{};
  virtual void OnMessage(const Naiive::Core::Message& msg) override{};

  void Initialize(ID3D11Device* device);
  void Shutdown();
  void Render(ID3D11Device* device, ID3D11DeviceContext* context,
              const std::string& text, const D3DXVECTOR2& pos, D3DXMATRIX proj);

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
}  // namespace Entity
}  // namespace PW

#endif
