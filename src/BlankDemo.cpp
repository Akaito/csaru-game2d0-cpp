#include "BlankDemo.hpp"

BlankDemo::BlankDemo(void)
{
}


BlankDemo::~BlankDemo(void)
{
}


bool BlankDemo::LoadContent(void)
{
  return true;
}


void BlankDemo::UnloadContent(void)
{
}


void BlankDemo::Update(float dt)
{
  dt;
}


void BlankDemo::Render(void)
{
  if (m_D3dContext == NULL)
    return;
  
  float clear_color[4] = {0.0f, 0.0f, 0.25f, 1.0f};
  m_D3dContext->ClearRenderTargetView(m_BackBufferTarget, clear_color);
  
  m_SwapChain->Present(0, 0);
}
