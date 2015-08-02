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

//#include "GraphicsMgr.h"

class Camera {
private: // Data
    XMFLOAT4X4 m_projectionFromWorldMtx; // aka view-projection matrix
    XMFLOAT2   m_position;
    float      m_width;
    float      m_height;
    float      m_nearZ;
    float      m_farZ;

public:
    Camera () {
        memset(this, 0, sizeof(*this));
    }

    void Setup (
        float viewLeft   =   0.0f,
        float viewRight  = 800.0f,
        float viewBottom =   0.0f,
        float viewTop    = 600.0f,
        float nearZ      =   0.1f,
        float farZ       = 100.0f
    ) {
        m_position.x = 0.0f;
        m_position.y = 0.0f;
        m_width      = viewRight - viewLeft;
        m_height     = viewTop - viewBottom;
        m_nearZ      = nearZ;
        m_farZ       = farZ;

        UpdateMatrix();
    }

    void UpdateMatrix () {
        float    halfWidth             = m_width  * 0.5f;
        float    halfHeight            = m_height * 0.5f;
        XMMATRIX viewMtx               = XMMatrixIdentity();
        XMMATRIX projectionFromViewMtx = XMMatrixOrthographicOffCenterLH(
            m_position.x - halfWidth,
            m_position.x + halfWidth,
            m_position.y - halfHeight,
            m_position.y + halfHeight,
            m_nearZ,
            m_farZ
        );

        XMMATRIX vpm_temp = XMMatrixMultiply(viewMtx, projectionFromViewMtx);
        memcpy(m_projectionFromWorldMtx.m, vpm_temp.m, sizeof(m_projectionFromWorldMtx));
    }

    const XMFLOAT2 & GetPosition () const { return m_position; }

    void SetXNoUpdate (float x) {
        m_position.x = x;
    }

    void SetYNoUpdate (float y) {
        m_position.y = y;
    }

    void SetX (float x) {
        m_position.x = x;
        UpdateMatrix();
    }

    void SetY (float y) {
        m_position.y = y;
        UpdateMatrix();
    }

    void SetPosition (float x, float y) {
        SetXNoUpdate(x);
        SetYNoUpdate(y);
        UpdateMatrix();
    }

    void SetPosition (const XMFLOAT2 & float2) {
        SetPositionNoUpdate(float2);
        UpdateMatrix();
    }

    void SetPositionNoUpdate (const XMFLOAT2 & float2) {
        SetXNoUpdate(float2.x);
        SetYNoUpdate(float2.y);
    }

    void GetProjectionFromWorldMtx (XMMATRIX * mtxOut) {
        memcpy(mtxOut->m, m_projectionFromWorldMtx.m, sizeof(mtxOut->m));
    }

};
