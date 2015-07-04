#pragma once

#include "Dx11DemoBase.hpp"

class TriangleDemo : public Dx11DemoBase
{
 public:
  TriangleDemo(void);
  virtual ~TriangleDemo(void);
  
  virtual bool LoadContent(void);
  virtual void UnloadContent(void);
  
  virtual void Update(float dt);
  virtual void Render(void);
 
 private:
  ID3D11VertexShader* m_SolidColorVS;
  ID3D11PixelShader* m_SolidColorPS;
  
  ID3D11InputLayout* m_InputLayout;
  ID3D11Buffer* m_VertexBuffer;
};
