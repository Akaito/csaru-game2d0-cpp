#include "TriangleDemo.hpp"
#include <xnamath.h>

struct VertexPos3Uv
{
  XMFLOAT3 pos;
};


TriangleDemo::TriangleDemo(void) :
    m_SolidColorVS(NULL), m_SolidColorPS(NULL),
    m_InputLayout(NULL), m_VertexBuffer(NULL)
{
}


TriangleDemo::~TriangleDemo(void)
{
}


bool TriangleDemo::LoadContent(void)
{
  ID3DBlob* vs_buffer = NULL;
  
  bool compile_result = CompileD3DShader("SolidGreenColor.fx", "VS_Main", "vs_4_0", &vs_buffer);
  
  if (compile_result == false)
  {
    MessageBox(NULL, L"Error loading vertex shader!", L"Compile Error", MB_OK);
    return false;
  }
  
  HRESULT d3d_result;
  
  d3d_result = m_D3dDevice->CreateVertexShader(
      vs_buffer->GetBufferPointer(), vs_buffer->GetBufferSize(),
      NULL,
      &m_SolidColorVS);
  
  if (FAILED(d3d_result))
  {
    if (vs_buffer)
      vs_buffer->Release();
    
    return false;
  }
  
  D3D11_INPUT_ELEMENT_DESC solid_color_layout[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };
  
  unsigned int layout_count = sizeof(solid_color_layout) / sizeof(solid_color_layout[0]);
  
  d3d_result = m_D3dDevice->CreateInputLayout(
      solid_color_layout, layout_count,
      vs_buffer->GetBufferPointer(), vs_buffer->GetBufferSize(),
      &m_InputLayout);
  
  vs_buffer->Release();
  
  if (FAILED(d3d_result))
  {
    return false;
  }
  
  ID3DBlob* ps_buffer = NULL;
  
  compile_result = CompileD3DShader("SolidGreenColor.fx", "PS_Main", "ps_4_0", &ps_buffer);
  
  if (compile_result == false)
  {
    MessageBox(NULL, L"Error loading pixel shader!", L"Compile Error", MB_OK);
    return false;
  }
  
  d3d_result = m_D3dDevice->CreatePixelShader(
      ps_buffer->GetBufferPointer(), ps_buffer->GetBufferSize(),
      NULL,
      &m_SolidColorPS);
  
  ps_buffer->Release();
  
  if (FAILED(d3d_result))
  {
    return false;
  }
  
  // ...
  
  VertexPos3Uv vertices[] =
  {
    XMFLOAT3( 0.5f,  0.5f, 0.5f),
    XMFLOAT3( 0.5f, -0.5f, 0.5f),
    XMFLOAT3(-0.5f, -0.5f, 0.5f)
  };
  
  D3D11_BUFFER_DESC vertex_desc;
  SecureZeroMemory(&vertex_desc, sizeof(vertex_desc));
  vertex_desc.Usage = D3D11_USAGE_DEFAULT;
  vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertex_desc.ByteWidth = sizeof(VertexPos3Uv) * 3;
  
  D3D11_SUBRESOURCE_DATA resource_data;
  SecureZeroMemory(&resource_data, sizeof(resource_data));
  resource_data.pSysMem = vertices;
  
  d3d_result = m_D3dDevice->CreateBuffer(&vertex_desc, &resource_data, &m_VertexBuffer);
  
  if (FAILED(d3d_result))
  {
    return false;
  }
  
  return true;
}


void TriangleDemo::UnloadContent(void)
{
  if (m_SolidColorVS)
    m_SolidColorVS->Release();
  if (m_SolidColorPS)
    m_SolidColorPS->Release();
  if (m_InputLayout)
    m_InputLayout->Release();
  if (m_VertexBuffer)
    m_VertexBuffer->Release();
  
  m_SolidColorVS = NULL;
  m_SolidColorPS = NULL;
  m_InputLayout = NULL;
  m_VertexBuffer = NULL;
}


void TriangleDemo::Update(float dt)
{
  dt;
}


void TriangleDemo::Render(void)
{
  if (m_D3dContext == NULL)
    return;
  
  float clear_color[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
  m_D3dContext->ClearRenderTargetView(m_BackBufferTarget, clear_color);
  
  unsigned int stride = sizeof(VertexPos3Uv);
  unsigned int offset = 0;
  
  m_D3dContext->IASetInputLayout(m_InputLayout);
  m_D3dContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
  m_D3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  
  m_D3dContext->VSSetShader(m_SolidColorVS, NULL, 0);
  m_D3dContext->PSSetShader(m_SolidColorPS, NULL, 0);
  m_D3dContext->Draw(3, 0);
  
  m_SwapChain->Present(0, 0);
}
