#pragma once

#include "Dx11DemoBase.hpp"

class TextureDemo : public Dx11DemoBase
{
 public:
  TextureDemo(void);
  virtual ~TextureDemo(void);
  
  virtual bool LoadContent(void);
  virtual void UnloadContent(void);
  
  virtual void Update(float dt);
  virtual void Render(void);
 
 private:
  ID3D11VertexShader* m_ColorMapVS;
  ID3D11PixelShader* m_ColorMapPS;
  
  ID3D11InputLayout* m_InputLayout;
  ID3D11Buffer* m_VertexBuffer;
  
  ID3D11ShaderResourceView* m_ColorMap;
  ID3D11SamplerState* m_ColorMapSampler;
};
