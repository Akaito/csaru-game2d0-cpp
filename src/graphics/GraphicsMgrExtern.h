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

struct VertexPos {
  XMFLOAT3 pos;
  XMFLOAT2 tex0;
};

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

};

extern IGraphicsMgr * g_graphicsMgr;
