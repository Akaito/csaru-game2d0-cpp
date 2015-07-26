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

#include "GraphicsMgr.h"

class DebugLine {
private: // Data
    VertexPos3Rgb  m_ends[2];
    VertexShader * m_vertexShader;
    PixelShader *  m_pixelShader;
    ID3D11Buffer * m_vertexBuffer;

public:
    DebugLine () {
        memset(this, 0, sizeof(*this));
    }

    void Render (const XMMATRIX & world, const XMMATRIX & viewProjection) {

        // Initial prep
        if (!m_vertexShader) {
            m_ends[0].pos.x = 0.0f;
            m_ends[0].pos.y = 0.0f;
            m_ends[0].pos.z = 0.0f;
            m_ends[0].rgb.x = 1.0f;
            m_ends[0].rgb.y = 1.0f;
            m_ends[0].rgb.z = 1.0f;

            m_ends[1].pos.x = 100.0f;
            m_ends[1].pos.y = 100.0f;
            m_ends[1].pos.z = 0.0f;
            m_ends[1].rgb.x = 1.0f;
            m_ends[1].rgb.y = 1.0f;
            m_ends[1].rgb.z = 1.0f;

            const wchar_t * debugLineShaderFilepath = L"SolidGreenColor.fx";
            m_vertexShader = g_graphicsMgrInternal->LoadVertexShader(
                L"DebugLine VS",
                debugLineShaderFilepath,
                "VS_Main"
            );
            m_pixelShader = g_graphicsMgrInternal->LoadPixelShader(
                L"DebugLine PS",
                debugLineShaderFilepath,
                "PS_Main"
            );

            // Vertex buffer
            {
                D3D11_BUFFER_DESC vertex_desc;
                SecureZeroMemory(&vertex_desc, sizeof(vertex_desc));
                vertex_desc.Usage = D3D11_USAGE_DEFAULT;
                vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                vertex_desc.ByteWidth = sizeof(VertexPos3Rgb) * 2;

                D3D11_SUBRESOURCE_DATA resource_data;
                SecureZeroMemory(&resource_data, sizeof(resource_data));
                resource_data.pSysMem = m_ends;

                HRESULT d3d_result = g_graphicsMgr->GetDevice()->CreateBuffer(&vertex_desc, &resource_data, &m_vertexBuffer);
                if (FAILED(d3d_result)) {
                    DXTRACE_MSG(L"Failed to create vertex buffer!");
                    ASSERT(0);
                }
                #if defined(_DEBUG)
                    char           tempName[256];
                    const unsigned strLen = sprintf_s(tempName, "DebugLine VertexBuffer");
                    m_vertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, strLen, tempName);
                #endif
            }
        }

        // Render prep
        {
            unsigned int stride = sizeof(VertexPos3Rgb);
            unsigned int offset = 0;

            ID3D11DeviceContext * d3dContext = g_graphicsMgrInternal->GetContext();
            d3dContext->IASetInputLayout(m_vertexShader->GetInputLayout());
            d3dContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
            d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

            d3dContext->VSSetShader(m_vertexShader->GetShader(), 0, 0);
            
            d3dContext->PSSetShader(m_pixelShader->GetShader(), 0, 0);
            //d3dContext->PSSetShaderResources(0, 1, &m_colorMap);
            //d3dContext->PSSetSamplers(0, 1, &m_colorMapSampler);
        }

        // Render
        {
            XMMATRIX mvp = XMMatrixMultiply(world, viewProjection);
            mvp = XMMatrixTranspose(mvp);
            
            ID3D11Buffer *        buffer     = g_graphicsMgrInternal->GetRenderResource();
            ID3D11DeviceContext * d3dContext = g_graphicsMgrInternal->GetContext();
            d3dContext->UpdateSubresource(buffer, 0, nullptr, &mvp, 0, 0);
            d3dContext->VSSetConstantBuffers(1, 1, &buffer);

            d3dContext->Draw(2, 0);
        }

    }

};
