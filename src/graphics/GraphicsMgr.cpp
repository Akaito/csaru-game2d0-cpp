/*
Copyright 2013 Christopher Higgins Barrett

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "GraphicsMgr.h"
#include <D3Dcompiler.h>
#include "../input/DirectInputKeyboardMouse.h"

//==============================================================================
CGraphicsMgr::CGraphicsMgr ()
    : m_driverType(D3D_DRIVER_TYPE_NULL),
      m_featureLevel(D3D_FEATURE_LEVEL_11_0),
      m_d3dDevice(NULL),
      m_d3dContext(NULL),
      m_swapChain(NULL),
      m_backBufferTarget(NULL),
      m_rasterState(NULL),
      m_mvpCB(NULL)
{
}

//==============================================================================
CGraphicsMgr::~CGraphicsMgr () {
    Shutdown();
}

//==============================================================================
bool CGraphicsMgr::CompileD3DShader (
    const std::wstring & filePath,
    const std::string &  entryFunction,
    const std::string &  shaderModel,
    ID3DBlob **          buffer
) {

    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* errorBuffer = NULL;
    HRESULT result;

    result = D3DX11CompileFromFileW(
        filePath.c_str(),
        NULL,
        NULL,
        entryFunction.c_str(),
        shaderModel.c_str(),
        shaderFlags,
        0,
        NULL,
        buffer,
        &errorBuffer,
        NULL
    );

    if (FAILED(result)) {
    
        if (errorBuffer != NULL) {
            OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
            errorBuffer->Release();
        }

        return false;
    }

    if (errorBuffer != NULL)
        errorBuffer->Release();

    return true;

}

//==============================================================================
PixelShader * CGraphicsMgr::FindPixelShader (const std::wstring & name) {

    std::vector<PixelShader>::iterator curr = m_pixelShaders.begin();
    std::vector<PixelShader>::iterator term = m_pixelShaders.end();
    for ( ; curr != term; ++curr) {
    
        if (curr->GetName() == name)
            return &*curr;
    }
    
    return NULL;

}

//==============================================================================
PixelShader * CGraphicsMgr::FindPixelShaderRaii (const std::wstring & name) {

    PixelShader * result = FindPixelShader(name);
    
    if (!result) {
        m_pixelShaders.push_back(PixelShader());
        result = &m_pixelShaders.back();
    }
    
    return result;

}

//==============================================================================
VertexShader * CGraphicsMgr::FindVertexShader (const std::wstring & name) {

    std::vector<VertexShader>::iterator curr = m_vertexShaders.begin();
    std::vector<VertexShader>::iterator term = m_vertexShaders.end();
    for ( ; curr != term; ++curr) {
    
        if (curr->GetName() == name)
            return &*curr;
    }
    
    return NULL;

}

//==============================================================================
VertexShader * CGraphicsMgr::FindVertexShaderRaii (const std::wstring & name) {

    VertexShader * result = FindVertexShader(name);
    
    if (!result) {
        m_vertexShaders.push_back(VertexShader());
        result = &m_vertexShaders.back();
    }
    
    return result;

}

//==============================================================================
void CGraphicsMgr::GetViewProjectionMatrix (XMMATRIX * mtxOut) {
    memcpy(mtxOut->r, m_ViewProjectionMatrix.m, sizeof(mtxOut->r));
}

//==============================================================================
PixelShader * CGraphicsMgr::LoadPixelShader (
    const std::wstring & name,
    const std::wstring & filepath,
    const std::string &  entryFunction
) {

    PixelShader * shader = FindPixelShaderRaii(name);
    
    if (!shader->Compile(name, filepath, entryFunction))
        return NULL;

    return shader;

}

//==============================================================================
VertexShader * CGraphicsMgr::LoadVertexShader (
    const std::wstring & name,
    const std::wstring & filepath,
    const std::string &  entryFunction
) {

    VertexShader * shader = FindVertexShaderRaii(name);
    
    if (!shader->Compile(name, filepath, entryFunction))
        return NULL;

    return shader;

}

//==============================================================================
void CGraphicsMgr::Shutdown () {

    if (g_keyboardMouse)
        g_keyboardMouse->Shutdown();

    if (m_mvpCB)
        m_mvpCB->Release();
    m_mvpCB = NULL;

    if (m_backBufferTarget)
        m_backBufferTarget->Release();
    m_backBufferTarget = NULL;
    
    if (m_swapChain)
        m_swapChain->Release();
    m_swapChain = NULL;
    
    if (m_d3dContext)
        m_d3dContext->Release();
    m_d3dContext = NULL;
    
    if (m_d3dDevice)
        m_d3dDevice->Release();
    m_d3dDevice = NULL;

}

//==============================================================================
bool CGraphicsMgr::Startup (HINSTANCE hInstance, HWND hwnd) {
    
    m_hInstance = hInstance;
    m_hWnd = hwnd;

    RECT dimensions;
    GetClientRect(hwnd, &dimensions);

    unsigned int width = dimensions.right - dimensions.left;
    unsigned int height = dimensions.bottom - dimensions.top;

    D3D_DRIVER_TYPE driver_types[] = {
        //D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_SOFTWARE
    };

    unsigned int driver_type_count = sizeof(driver_types) / sizeof(driver_types[0]);

    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    unsigned int feature_level_count = sizeof(feature_levels) / sizeof(feature_levels[0]);

    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    SecureZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));
    swap_chain_desc.BufferCount = 1;
    swap_chain_desc.BufferDesc.Width = width;
    swap_chain_desc.BufferDesc.Height = height;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = hwnd;
    swap_chain_desc.Windowed = true;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;

    unsigned int creation_flags = 0;

#if defined(_DEBUG)
    creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hresult;
    unsigned int driver;

    for (driver = 0;  driver < driver_type_count;  ++driver) {
    
        hresult = D3D11CreateDeviceAndSwapChain(
            0, driver_types[driver], 0,
            creation_flags,
            feature_levels, feature_level_count,
            D3D11_SDK_VERSION,
            &swap_chain_desc, &m_swapChain, &m_d3dDevice, &m_featureLevel, &m_d3dContext
        );

        if (SUCCEEDED(hresult)) {
            m_driverType = driver_types[driver];
            break;
        }
    }

    if (FAILED(hresult)) {
        DXTRACE_MSG(L"Failed to create the Direct3D device!");
        return false;
    }

    ID3D11Texture2D* back_buffer_texture;
    hresult = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer_texture);

    if (FAILED(hresult)) {
        DXTRACE_MSG(L"Failed to get the swap chain back buffer!");
        return false;
    }

    hresult = m_d3dDevice->CreateRenderTargetView(back_buffer_texture, 0, &m_backBufferTarget);

    if (back_buffer_texture)
    back_buffer_texture->Release();

    if (FAILED(hresult)) {
        DXTRACE_MSG(L"Failed to create the render target view!");
        ASSERT(!FAILED(hresult));
        return false;
    }

    m_d3dContext->OMSetRenderTargets(1, &m_backBufferTarget, 0);

    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    m_d3dContext->RSSetViewports(1, &viewport);
    
    // Don't backface cull
    {
        D3D11_RASTERIZER_DESC rasterDesc;
        SecureZeroMemory(&rasterDesc, sizeof(rasterDesc));
        rasterDesc.CullMode        = D3D11_CULL_NONE;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.FillMode        = D3D11_FILL_SOLID;
        //rasterDesc.FillMode        = D3D11_FILL_WIREFRAME;
        
        HRESULT result = m_d3dDevice->CreateRasterizerState(
            &rasterDesc,
            &m_rasterState
        );
        
        if (FAILED(result)) {
            ASSERT(!FAILED(result));
            return false;
        }
            
        m_d3dContext->RSSetState(m_rasterState);
    }
    
    // TODO : Move this to a camera class
    XMMATRIX view = XMMatrixIdentity();
    XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);

    XMMATRIX vpm_temp = XMMatrixMultiply(view, projection);
    memcpy(m_ViewProjectionMatrix.m, vpm_temp.m, sizeof(m_ViewProjectionMatrix));
    
    
    D3D11_BUFFER_DESC const_desc;
    SecureZeroMemory(&const_desc, sizeof(const_desc));
    const_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    const_desc.ByteWidth = sizeof(XMMATRIX);
    const_desc.Usage = D3D11_USAGE_DEFAULT;

    HRESULT d3d_result = g_graphicsMgr->GetDevice()->CreateBuffer(&const_desc, 0, &m_mvpCB);

    if (FAILED(d3d_result)) {
        ASSERT(!FAILED(d3d_result));
        return false;
    }
        
    //m_vertexShaders.reserve(16);
    //m_pixelShaders.reserve(16);
    
    ASSERT(!g_keyboardMouse);
    XInputKeyboardMouse * xikm = new XInputKeyboardMouse();
    xikm->Startup(hInstance, hwnd, width, height);
    
    return true;

}

//==============================================================================
void CGraphicsMgr::RenderPre () {

    float clear_color[4] = {0.0f, 0.0f, 0.25f, 1.0f};
    m_d3dContext->ClearRenderTargetView(m_backBufferTarget, clear_color);

}

//==============================================================================
void CGraphicsMgr::RenderPost () {

    m_swapChain->Present(1, 0);

}

//==============================================================================
//
// Exported functions
//
//==============================================================================

//==============================================================================
void IGraphicsMgr::Shutdown () {

    if (!g_graphicsMgr)
        return;
        
    g_graphicsMgrInternal->Shutdown();
        
    delete g_graphicsMgr;
    g_graphicsMgr = g_graphicsMgrInternal = NULL;

}

//==============================================================================
bool IGraphicsMgr::Startup (HINSTANCE hInstance, HWND hwnd) {

    if (g_graphicsMgr)
        return false;
        
    g_graphicsMgr = g_graphicsMgrInternal = new CGraphicsMgr();
    
    return g_graphicsMgrInternal->Startup(hInstance, hwnd);

}

//==============================================================================
IGraphicsMgr * g_graphicsMgr         = NULL;
CGraphicsMgr * g_graphicsMgrInternal = NULL;
