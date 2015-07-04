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

#pragma once
#include "GraphicsMgrExtern.h"
#include "VertexShader.h"
#include "PixelShader.h"

class CGraphicsMgr : public IGraphicsMgr {

private: // Data

    HINSTANCE m_hInstance;
    HWND      m_hWnd;

    D3D_DRIVER_TYPE   m_driverType;
    D3D_FEATURE_LEVEL m_featureLevel;

    ID3D11Device *           m_d3dDevice;
    ID3D11DeviceContext *    m_d3dContext;
    IDXGISwapChain *         m_swapChain;
    ID3D11RenderTargetView * m_backBufferTarget;
    ID3D11RasterizerState *  m_rasterState;
    
    XMFLOAT4X4 m_ViewProjectionMatrix;
    ID3D11Buffer* m_mvpCB;
    
    std::vector<VertexShader> m_vertexShaders;
    std::vector<PixelShader>  m_pixelShaders;

private: // Helpers

    VertexShader * FindVertexShaderRaii (const std::wstring & name);
    PixelShader *  FindPixelShaderRaii (const std::wstring & name);
    bool           PrepareInputLayout ();
    bool           fooVertexBuffer ();

public: // Construction

    CGraphicsMgr ();
    virtual ~CGraphicsMgr ();
    
public: // Internal interface

    bool Startup (HINSTANCE hInstance, HWND hwnd);
    void Shutdown ();
    
    bool CompileD3DShader (
        const std::wstring & filePath,
        const std::string &  entryFunction,
        const std::string &  shaderModel,
        ID3DBlob **          buffer
    );
    
    VertexShader * FindVertexShader (const std::wstring & name);
    PixelShader *  FindPixelShader (const std::wstring & name);
    
    void GetViewProjectionMatrix (XMMATRIX * mtxOut);
    ID3D11Buffer * GetRenderResource () { return m_mvpCB; }; // TODO : Just what is this thing?
    
public: // IGraphicsMgr
    
    ID3D11Device *        GetDevice () { return m_d3dDevice; }
    ID3D11DeviceContext * GetContext () { return m_d3dContext; }
    
    void RenderPre ();
    void RenderPost ();
    
    VertexShader * LoadVertexShader (
        const std::wstring & name,
        const std::wstring & filepath,
        const std::string &  entryFunction
    );
    PixelShader * LoadPixelShader (
        const std::wstring & name,
        const std::wstring & filepath,
        const std::string &  entryFunction
    );

};

extern CGraphicsMgr * g_graphicsMgrInternal;
