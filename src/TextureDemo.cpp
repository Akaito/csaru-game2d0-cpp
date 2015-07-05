#include "TextureDemo.hpp"
#include <xnamath.h>

struct VertexPos3Uv
{
  XMFLOAT3 pos;
  XMFLOAT2 tex0;
};


TextureDemo::TextureDemo(void) :
    m_ColorMapVS(NULL), m_ColorMapPS(NULL),
    m_InputLayout(NULL), m_VertexBuffer(NULL),
    m_ColorMap(NULL), m_ColorMapSampler(NULL)
{
}


TextureDemo::~TextureDemo(void)
{
}


bool TextureDemo::LoadContent(void)
{
  //
  // Load vertex Shader
  //
  
  ID3DBlob* vs_buffer = NULL;
  
  bool compile_result = CompileD3DShader("ColorMap.fx", "VS_Main", "vs_4_0", &vs_buffer);
  
  if (compile_result == false)
  {
    MessageBox(NULL, L"Error loading vertex shader!", L"Compile Error", MB_OK);
    return false;
  }
  
  HRESULT d3d_result;
  
  d3d_result = m_D3dDevice->CreateVertexShader(
      vs_buffer->GetBufferPointer(), vs_buffer->GetBufferSize(),
      NULL,
      &m_ColorMapVS);
  
  if (FAILED(d3d_result))
  {
    if (vs_buffer)
      vs_buffer->Release();
    
    return false;
  }
  
  D3D11_INPUT_ELEMENT_DESC solid_color_layout[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    //{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPos3Uv,tex0), D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
  
  //
  // Load Pixel Shader
  //
  
  ID3DBlob* ps_buffer = NULL;
  
  compile_result = CompileD3DShader("ColorMap.fx", "PS_Main", "ps_4_0", &ps_buffer);
  
  if (compile_result == false)
  {
    MessageBox(NULL, L"Error loading pixel shader!", L"Compile Error", MB_OK);
    return false;
  }
  
  d3d_result = m_D3dDevice->CreatePixelShader(
      ps_buffer->GetBufferPointer(), ps_buffer->GetBufferSize(),
      NULL,
      &m_ColorMapPS);
  
  ps_buffer->Release();
  
  if (FAILED(d3d_result))
  {
    return false;
  }
  
  // ...
  
  VertexPos3Uv vertices[] =
  {
    { XMFLOAT3( 1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
    
    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(-1.0f,  1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) }
  };
  
  //
  // Create Vertex Buffer
  //
  
  D3D11_BUFFER_DESC vertex_desc;
  SecureZeroMemory(&vertex_desc, sizeof(vertex_desc));
  vertex_desc.Usage = D3D11_USAGE_DEFAULT;
  vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertex_desc.ByteWidth = sizeof(VertexPos3Uv) * 6;
  
  D3D11_SUBRESOURCE_DATA resource_data;
  SecureZeroMemory(&resource_data, sizeof(resource_data));
  resource_data.pSysMem = vertices;
  
  d3d_result = m_D3dDevice->CreateBuffer(&vertex_desc, &resource_data, &m_VertexBuffer);
  
  if (FAILED(d3d_result))
  {
    return false;
  }
  
  // new thing is what?
  
  d3d_result = D3DX11CreateShaderResourceViewFromFile(
      m_D3dDevice, L"plain_kirby.png", NULL, NULL, &m_ColorMap, NULL);
  
  if (FAILED(d3d_result))
  {
    DXTRACE_MSG(L"Failed to load the texture image!");
    return false;
  }
  
  D3D11_SAMPLER_DESC color_map_desc;
  SecureZeroMemory(&color_map_desc, sizeof(color_map_desc));
  color_map_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  color_map_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  color_map_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  color_map_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  //color_map_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  color_map_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  color_map_desc.MaxLOD = D3D11_FLOAT32_MAX;
  
  d3d_result = m_D3dDevice->CreateSamplerState(&color_map_desc, &m_ColorMapSampler);
  
  if (FAILED(d3d_result))
  {
    DXTRACE_MSG(L"Failed to create color map sampler state!");
    return false;
  }
  
  return true;
}


void TextureDemo::UnloadContent(void)
{
  if (m_ColorMapSampler)
    m_ColorMapSampler->Release();
  if (m_ColorMap)
    m_ColorMap->Release();
  if (m_ColorMapVS)
    m_ColorMapVS->Release();
  if (m_ColorMapPS)
    m_ColorMapPS->Release();
  if (m_InputLayout)
    m_InputLayout->Release();
  if (m_VertexBuffer)
    m_VertexBuffer->Release();
  
  m_ColorMapSampler = NULL;
  m_ColorMap = NULL;
  m_ColorMapVS = NULL;
  m_ColorMapPS = NULL;
  m_InputLayout = NULL;
  m_VertexBuffer = NULL;
}


void TextureDemo::Update(float dt)
{
  dt;
}


void TextureDemo::Render(void)
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
  
  m_D3dContext->VSSetShader(m_ColorMapVS, NULL, 0);
  m_D3dContext->PSSetShader(m_ColorMapPS, NULL, 0);
  m_D3dContext->PSSetShaderResources(0, 1, &m_ColorMap);
  m_D3dContext->PSSetSamplers(0, 1, &m_ColorMapSampler);
  m_D3dContext->Draw(6, 0);
  
  m_SwapChain->Present(0, 0);
}
