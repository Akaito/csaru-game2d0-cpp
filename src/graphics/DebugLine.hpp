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
    ID3D11Buffer * m_perObjectCb;

    struct PerObjectCbData {
        XMFLOAT4X4 mtx;
    };

    // Helpers
    bool PrepareConstantBuffers () {

        // Supply initial data
        PerObjectCbData cbPerObjectData;
        SecureZeroMemory(&cbPerObjectData, sizeof(cbPerObjectData));

        D3D11_BUFFER_DESC cbPerObjectDesc;
        SecureZeroMemory(&cbPerObjectDesc, sizeof(cbPerObjectDesc));
        // ByteWidth must be a multiple of 16
        // TODO : Is it okay to just lie like this?
        cbPerObjectDesc.ByteWidth           = sizeof(cbPerObjectData) + (sizeof(cbPerObjectData) % 16);
        cbPerObjectDesc.Usage               = D3D11_USAGE_DEFAULT;
        cbPerObjectDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        //cbPerObjectDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        
        // Subresource data
        D3D11_SUBRESOURCE_DATA initData;
        SecureZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = &cbPerObjectData;
        
        HRESULT d3dResult = g_graphicsMgr->GetDevice()->CreateBuffer(
            &cbPerObjectDesc,
            &initData,
            &m_perObjectCb
        );
        if (FAILED(d3dResult)) {
            DXTRACE_MSG(L"Failed to create constant buffer!");
            return false;
        }
    #if defined(_DEBUG)
        char           tempName[256];
        const unsigned strLen = sprintf_s(tempName, "DebugLine PerObjectConstantBuffer");
        m_perObjectCb->SetPrivateData(WKPDID_D3DDebugObjectName, strLen, tempName);
    #endif
        
        return true;

    }

public:
    DebugLine () {
        memset(this, 0, sizeof(*this));
    }

    ~DebugLine () {
        if (m_perObjectCb) {
            m_perObjectCb->Release();
            m_perObjectCb = nullptr;
        }

        if (m_vertexBuffer) {
            m_vertexBuffer->Release();
            m_vertexBuffer = nullptr;
        }
    }

    VertexPos3Rgb * Ends () { return m_ends; }

    void UpdateVertexBuffer () {

        if (!m_vertexBuffer)
            return;

        ID3D11DeviceContext *    d3dContext = g_graphicsMgrInternal->GetContext();
        D3D11_MAPPED_SUBRESOURCE resource;
        d3dContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        memcpy(resource.pData, m_ends, sizeof(m_ends));
        d3dContext->Unmap(m_vertexBuffer, 0);

    }

    void Render (const XMMATRIX & worldFromModelMtx) {

        // Initial prep
        if (!m_vertexShader) {
            m_ends[0].pos.x = 0.0f;
            m_ends[0].pos.y = 0.0f;
            m_ends[0].pos.z = 1.0f;
            m_ends[0].rgb.x = 1.0f;
            m_ends[0].rgb.y = 1.0f;
            m_ends[0].rgb.z = 1.0f;

            m_ends[1].pos.x = 100.5f;
            m_ends[1].pos.y = 0.1f;
            m_ends[1].pos.z = 1.0f;
            m_ends[1].rgb.x = 1.0f;
            m_ends[1].rgb.y = 0.0f;
            m_ends[1].rgb.z = 1.0f;

            const wchar_t * debugLineShaderFilepath = L"ColoredVerts.fx";
            m_vertexShader = g_graphicsMgrInternal->LoadVertexShader(
                L"DebugLine VS",
                debugLineShaderFilepath,
                "VS_Main",
                s_VertexPos3RgbLayout,
                s_VertexPos3RgbLayoutCount
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
                vertex_desc.Usage     = D3D11_USAGE_DYNAMIC;
                vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                vertex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
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

            PrepareConstantBuffers();
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
        }

        // Render
        {
            ID3D11DeviceContext * d3dContext = g_graphicsMgrInternal->GetContext();
            //*
            XMMATRIX transWorldFromModel = XMMatrixTranspose(worldFromModelMtx);
            
            d3dContext->UpdateSubresource(m_perObjectCb, 0, nullptr, &transWorldFromModel, 0, 0);
            d3dContext->VSSetConstantBuffers(1, 1, &m_perObjectCb);
            //*/

            d3dContext->Draw(2, 0);
        }

    }

};
