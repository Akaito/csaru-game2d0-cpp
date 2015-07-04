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

#include "VertexShader.h"

//==============================================================================
VertexShader::VertexShader ()
    :   m_shader(NULL),
        m_InputLayout(NULL)
{
}

//==============================================================================
VertexShader::~VertexShader () {

    if (m_InputLayout)
        m_InputLayout->Release();
    m_InputLayout = NULL;

    if (m_shader)
        m_shader->Release();
    m_shader = NULL;

}

//==============================================================================
bool VertexShader::Compile (
    const std::wstring & name,
    const std::wstring & filepath,
    const std::string &  entryFunction
) {
    
    m_name = name;

    ID3DBlob * vsBuffer      = NULL;
    bool       compileResult = g_graphicsMgrInternal->CompileD3DShader(
        filepath,
        entryFunction,
        "vs_4_0",
        &vsBuffer
    );
    ASSERT(compileResult && "Error compiling vertex shader.");

    HRESULT d3dResult;

    d3dResult = g_graphicsMgr->GetDevice()->CreateVertexShader(
        vsBuffer->GetBufferPointer(),
        vsBuffer->GetBufferSize(),
        NULL,
        &m_shader
    );

    if (FAILED(d3dResult)) {
        if (vsBuffer)
          vsBuffer->Release();

        return false;
    }
    
    //
    // Prepare input layout
    //

    D3D11_INPUT_ELEMENT_DESC solid_color_layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    unsigned int layout_count = sizeof(solid_color_layout) / sizeof(solid_color_layout[0]);

    d3dResult = g_graphicsMgrInternal->GetDevice()->CreateInputLayout(
        solid_color_layout,
        layout_count,
        vsBuffer->GetBufferPointer(),
        vsBuffer->GetBufferSize(),
        &m_InputLayout
    );

    vsBuffer->Release();

    if (FAILED(d3dResult))
        return false;
        
    return true;

}
