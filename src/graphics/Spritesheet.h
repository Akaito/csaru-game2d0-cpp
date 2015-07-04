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
#include "../CsDataMap/DataMapReaderSimple.h"
#include "GraphicsMgr.h"

class Spritesheet {

public: // Constants and types

    struct Frame {
        unsigned short x;
        unsigned short y;
        unsigned short width;
        unsigned short height;
        unsigned short durationMs;
        
        Frame();
        bool FromDataMap (const Core::DataMapReaderSimple & reader);
    };

    struct Animation {
        std::wstring        name;
        std::vector<Frame> frames;
        
        Animation();
        bool FromDataMap (const CSaruContainer::DataMapReader & reader);
    };
    
    struct VertexShaderPerFrameConstantBufferData {
        XMFLOAT4X4 projectionFromModel; // TODO : Lie about what this is, and pass in mvp to the shader anyway.  Make sure we can get just one cb working.
    };
    
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
