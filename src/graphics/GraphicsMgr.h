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

#pragma once
#include "GraphicsMgrExtern.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Camera.hpp"

class CGraphicsMgr : public IGraphicsMgr {

public: // Types and Constants

    struct DebugLines {
        VertexPos3Rgb  points[42];
        unsigned       lineCount;
    };

private: // Data

    HINSTANCE m_hInstance;
    HWND      m_hWnd;

    D3D_DRIVER_TYPE   m_driverType;
    D3D_FEATURE_LEVEL m_featureLevel;

    ID3D11Device *           m_d3dDevice;
    ID3D11DeviceContext *    m_d3dContext;
    ID3D11Debug *            m_d3dDebug;
    IDXGISwapChain *         m_swapChain;
    ID3D11RenderTargetView * m_backBufferTarget;
    ID3D11RasterizerState *  m_rasterState;
    
    Camera * m_camera;
    ID3D11Buffer* m_projectionFromWorldMtxCb;
    
    std::vector<VertexShader> m_vertexShaders;
    std::vector<PixelShader>  m_pixelShaders;

    std::map<std::string, Spritesheet *> m_spritesheets;

private: // Helpers

    VertexShader * FindVertexShaderRaii (const std::wstring & name);
    PixelShader *  FindPixelShaderRaii (const std::wstring & name);

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
    
    //ID3D11Buffer * GetProjectionFromWorldMtxCb () { return m_projectionFromWorldMtxCb; };
    
public: // IGraphicsMgr
    
    ID3D11Device *        GetDevice () override { return m_d3dDevice; }
    ID3D11DeviceContext * GetContext () override { return m_d3dContext; }
    
    void RenderPre () override;
    void RenderPost () override;
    
    VertexShader * LoadVertexShader (
        const std::wstring & name,
        const std::wstring & filepath,
        const std::string &  entryFunction
    ) override;
    PixelShader * LoadPixelShader (
        const std::wstring & name,
        const std::wstring & filepath,
        const std::string &  entryFunction
    ) override;

    Spritesheet * LoadSpritesheet (const char * filepath) override;

    Camera * GetActiveCamera () override { return m_camera; }
    void     SetActiveCamera (Camera * camera) override { m_camera = camera; }

};

extern CGraphicsMgr * g_graphicsMgrInternal;
