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

class Spritesheet;
class Camera;

// 3D Position and UV texture coordinates
struct VertexPos3Uv {
    XMFLOAT3 pos;
    XMFLOAT2 tex0;
};
extern const D3D11_INPUT_ELEMENT_DESC s_VertexPos3UvLayout[];
extern const unsigned                 s_VertexPos3UvLayoutCount;

// 3D Position and RGB float color
struct VertexPos3Rgb {
    XMFLOAT3 pos;
    XMFLOAT3 rgb;
};
extern const D3D11_INPUT_ELEMENT_DESC s_VertexPos3RgbLayout[];
extern const unsigned                 s_VertexPos3RgbLayoutCount;


struct IVertexShader {
    virtual ~IVertexShader () {};
};

struct IPixelShader {
    virtual ~IPixelShader () {};
};

struct IGraphicsMgr {

public: // Queries
    
    virtual ID3D11Device *        GetDevice () = 0;
    virtual ID3D11DeviceContext * GetContext () = 0;
   
public: // Commands

    static bool Startup (HINSTANCE hInstance, HWND hwnd);
    static void Shutdown ();
    
    virtual void RenderPre () = 0;
    virtual void RenderPost () = 0;
    
    virtual IVertexShader * LoadVertexShader (
        const std::wstring & name,
        const std::wstring & filepath,
        const std::string &  entryFunction
    ) = 0;
    virtual IPixelShader * LoadPixelShader (
        const std::wstring & name,
        const std::wstring & filepath,
        const std::string &  entryFunction
    ) = 0;

    virtual Spritesheet * LoadSpritesheet (const char * filepath) = 0;

    virtual Camera * GetActiveCamera () = 0;
    virtual void     SetActiveCamera (Camera * camera) = 0;

};

extern IGraphicsMgr * g_graphicsMgr;
