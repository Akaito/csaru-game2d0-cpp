/*
The MIT License (MIT)

Copyright (c) 2015 Christopher Higgins Barrett

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "GraphicsMgr.h"
#include "Spritesheet.h"
#include <D3Dcompiler.h>
#include "../input/DirectInputKeyboardMouse.h"

#define REPORT_OBJECT_LEAKS 1

//==============================================================================
const D3D11_INPUT_ELEMENT_DESC s_VertexPos3UvLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, sizeof(XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
const unsigned s_VertexPos3UvLayoutCount = arrsize(s_VertexPos3UvLayout);

//==============================================================================
const D3D11_INPUT_ELEMENT_DESC s_VertexPos3RgbLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
const unsigned s_VertexPos3RgbLayoutCount = arrsize(s_VertexPos3RgbLayout);

//==============================================================================
CGraphicsMgr::CGraphicsMgr ()
    : m_driverType(D3D_DRIVER_TYPE_NULL),
      m_featureLevel(D3D_FEATURE_LEVEL_11_0),
      m_d3dDevice(nullptr),
      m_d3dContext(nullptr),
      m_d3dDebug(nullptr),
      m_swapChain(nullptr),
      m_backBufferTarget(nullptr),
      m_rasterState(nullptr),
      m_mvpCB(nullptr)
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
Spritesheet * CGraphicsMgr::LoadSpritesheet (const char * filepath) {

    std::map<std::string, Spritesheet *>::iterator iter = m_spritesheets.find(filepath);
    if (iter != m_spritesheets.end())
        return iter->second;

    Spritesheet * sheet = new Spritesheet;
    if (!sheet->BuildFromDatafile(filepath)) {
        delete sheet;
        return nullptr;
    }

    m_spritesheets[filepath] = sheet;
    return sheet;

}

//==============================================================================
VertexShader * CGraphicsMgr::LoadVertexShader (
    const std::wstring & name,
    const std::wstring & filepath,
    const std::string &  entryFunction
) {

    VertexShader * shader = FindVertexShaderRaii(name);
    
    if (!shader->Compile(name, filepath, entryFunction))
        return nullptr;

    return shader;

}

//==============================================================================
void CGraphicsMgr::Shutdown () {

    // spritesheets
    {
        for (std::pair<std::string, Spritesheet *> sheetIter : m_spritesheets)
            delete sheetIter.second;
        m_spritesheets.clear();
    }

    // vertex shaders
    m_vertexShaders.clear();
    m_pixelShaders.clear();

    if (g_keyboardMouse)
        g_keyboardMouse->Shutdown();

    if (m_mvpCB)
        m_mvpCB->Release();
    m_mvpCB = nullptr;

    if (m_backBufferTarget)
        m_backBufferTarget->Release();
    m_backBufferTarget = nullptr;
    
    if (m_swapChain)
        m_swapChain->Release();
    m_swapChain = nullptr;

    if (m_rasterState)
        m_rasterState->Release();
    m_rasterState = nullptr;
    
    if (m_d3dContext)
        m_d3dContext->Release();
    m_d3dContext = nullptr;

    if (m_d3dDebug) {
#if REPORT_OBJECT_LEAKS
        m_d3dDebug->ReportLiveDeviceObjects(
            D3D11_RLDO_SUMMARY |
            D3D11_RLDO_DETAIL
        );
#endif
        m_d3dDebug->Release();
    }
    m_d3dDebug = nullptr;
    
    if (m_d3dDevice)
        m_d3dDevice->Release();
    m_d3dDevice = nullptr;

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
#if defined(_DEBUG)
    {
        char tempName[] = "GraphicsMgr's SwapChain";
        m_swapChain->SetPrivateData(WKPDID_D3DDebugObjectName, arrsize(tempName), tempName);
    }
#endif

    // Debug layer
    if (SUCCEEDED(m_d3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_d3dDebug))) {
        ID3D11InfoQueue * d3dInfoQueue = nullptr;
        if (SUCCEEDED(m_d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue))) {
#if defined(_DEBUG)
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
            //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, true);
#endif

            D3D11_MESSAGE_ID hiddenMessages[] = {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
            };

            D3D11_INFO_QUEUE_FILTER filter;
            memset(&filter, 0, sizeof(filter));
            filter.DenyList.NumIDs  = _countof(hiddenMessages);
            filter.DenyList.pIDList = hiddenMessages;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
            d3dInfoQueue->Release();
        }
    }

    ID3D11Texture2D* back_buffer_texture;
    hresult = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer_texture);

    if (FAILED(hresult)) {
        DXTRACE_MSG(L"Failed to get the swap chain back buffer!");
        return false;
    }

    hresult = m_d3dDevice->CreateRenderTargetView(back_buffer_texture, 0, &m_backBufferTarget);
    if (FAILED(hresult)) {
        DXTRACE_MSG(L"Failed to create the render target view!");
        ASSERT(!FAILED(hresult));
        return false;
    }
#if defined(_DEBUG)
    {
        char tempName[] = "GraphicsMgr's BackBufferTarget";
        m_backBufferTarget->SetPrivateData(WKPDID_D3DDebugObjectName, arrsize(tempName), tempName);
    }
#endif

    if (back_buffer_texture)
        back_buffer_texture->Release();

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
#if defined(_DEBUG)
        {
            char tempName[] = "GraphicsMgr's RasterState";
            m_rasterState->SetPrivateData(WKPDID_D3DDebugObjectName, arrsize(tempName), tempName);
        }
#endif
            
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
#if defined(_DEBUG)
    {
        char tempName[] = "GraphicsMgr's model-view-proj ConstantBuffer";
        m_mvpCB->SetPrivateData(WKPDID_D3DDebugObjectName, arrsize(tempName), tempName);
    }
#endif
        
#ifdef _DEBUG
    // Hacks!! Hides issues with lost pointers to VertexShaders!
    m_vertexShaders.reserve(16);
    m_pixelShaders.reserve(16);
#endif
    
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
