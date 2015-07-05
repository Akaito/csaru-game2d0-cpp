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

#include "PixelShader.h"

//==============================================================================
PixelShader::PixelShader () :
    m_shader(NULL)
{}

//==============================================================================
PixelShader::~PixelShader () {

    if (m_shader)
        m_shader->Release();
    m_shader = NULL;

}

//==============================================================================
bool PixelShader::Compile (
    const std::wstring & name,
    const std::wstring & filepath,
    const std::string &  entryFunction
) {
    
    m_name = name;

    ID3DBlob * psBuffer      = NULL;
    bool       compileResult = g_graphicsMgrInternal->CompileD3DShader(
        filepath,
        entryFunction,
        "ps_4_0",
        &psBuffer
    );
    ASSERT(compileResult && "Error compiling pixel shader.");
    
    HRESULT d3dResult;

    d3dResult = g_graphicsMgr->GetDevice()->CreatePixelShader(
        psBuffer->GetBufferPointer(),
        psBuffer->GetBufferSize(),
        NULL,
        &m_shader
    );
    if (FAILED(d3dResult))
        return false;
#if defined(_DEBUG)
    char     tempName[256];
    unsigned tempNameLen = sprintf_s(tempName, "PixelShader {%S}'s PixelShader", name.c_str());
    m_shader->SetPrivateData(WKPDID_D3DDebugObjectName, tempNameLen, tempName);
#endif
    
    psBuffer->Release();
    
    return true;

}

