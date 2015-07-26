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
#include <DataMapReaderSimple.hpp>
#include "GraphicsMgr.h"

struct SpritesheetFrame {
    unsigned short x;
    unsigned short y;
    unsigned short width;
    unsigned short height;
    unsigned short durationMs;
    
    SpritesheetFrame();
    bool FromDataMap (const CSaruContainer::DataMapReaderSimple & reader);
};


class Spritesheet {

public: // Constants and types

    struct Animation {
        std::wstring                  name;
        std::vector<SpritesheetFrame> frames;
        
        Animation();
        bool FromDataMap (const CSaruContainer::DataMapReader & reader);
    };
    
    /*
    struct VertexShaderPerFrameConstantBufferData {
        XMFLOAT4X4 projectionFromModel; // TODO : Lie about what this is, and pass in mvp to the shader anyway.  Make sure we can get just one cb working.
    };
    //*/
    
    struct VertexShaderPerObjectConstantBufferData {
        //XMFLOAT4X4 worldFromModel;
        XMFLOAT2   textureDims;  // texels
        XMFLOAT2   frameTexPos;  // upper-left corner of frame in texels
        XMFLOAT2   frameTexDims; // width/height of frame in texels
    };

private: // Data

    std::wstring            m_name;
    std::wstring            m_sourceFilepath;
    std::wstring            m_imageFilepath;
    unsigned                m_textureWidth;
    unsigned                m_textureHeight;
    std::vector<Animation> m_animations;
    
    VertexShader * m_vertexShader;
    PixelShader *  m_pixelShader;

    ID3D11Buffer * m_vertexBuffer;
    ID3D11Buffer * m_perObjectCb;

    ID3D11ShaderResourceView* m_colorMap;
    ID3D11SamplerState* m_colorMapSampler;
    ID3D11BlendState* m_alphaBlendState;
    
private: // Helpers

    bool PrepareGraphicsResources ();
    bool PrepareVertexBuffer ();
    bool PrepareConstantBuffers ();
    bool PrepareBlendState ();
    
public: // Construction

    Spritesheet ();
    virtual ~Spritesheet ();
    
public: // Queries

    const Animation * GetAnimation (unsigned index) const;
    const Animation * GetAnimation (const std::wstring & name) const;
    unsigned          GetAnimationIndex (const std::wstring & name) const;

    const std::wstring & GetName () const { return m_name; }

public: // Commands

    bool BuildFromDatafile (const char * filepath);
    bool RebuildFromDatafile ();
    void RenderPrep (
        unsigned u,
        unsigned v,
        unsigned width,
        unsigned height
    );
    
    void Reset();

};
