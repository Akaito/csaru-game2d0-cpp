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

#include "PixelShader.h"

//==============================================================================
PixelShader::PixelShader ()
    : m_shader(NULL)
{
}

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
    
    psBuffer->Release();

    if (FAILED(d3dResult))
        return false;
    
    return true;

}

