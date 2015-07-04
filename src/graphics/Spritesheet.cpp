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

#include "Spritesheet.h"
#include <DataMap.hpp>
#include <JsonParserCallbackForDataMap.hpp>
#include <shlwapi.h>
#include <DataMapReaderSimple.hpp>

//==============================================================================
Spritesheet::Frame::Frame ()
    : x(0),
      y(0),
      width(0),
      height(0),
      durationMs(0)
{
}

//==============================================================================
bool Spritesheet::Frame::FromDataMap (const CSaruContainer::DataMapReaderSimple & reader) {

    x          = unsigned short(reader.Int("x"));
    y          = unsigned short(reader.Int("y"));
    width      = unsigned short(reader.Int("width"));
    height     = unsigned short(reader.Int("height"));
    durationMs = unsigned short(reader.Int("durationMs", durationMs));
    
    return width && height;

}

//==============================================================================
Spritesheet::Animation::Animation () {
}

//==============================================================================
bool Spritesheet::Animation::FromDataMap (const CSaruContainer::DataMapReader & reader) {

    CSaruContainer::DataMapReaderSimple animReader(reader);
    
    name = animReader.WString("name");
    
    // Read in each frame's data
    for (animReader.EnterArray("frames"); animReader.IsValid(); animReader.ToNextSibling()) {
    
        Frame frame;
        if (!frame.FromDataMap(animReader))
            continue;
            
        frames.push_back(frame);
        
    }
    animReader.ExitArray();
    
    if (frames.empty()) {
        ASSERT(0 && "Animation has no frame data.");
        return false;
    }
    
    return true;

}

//==============================================================================
Spritesheet::Spritesheet () :
    m_textureWidth(0),
    m_textureHeight(0),
    m_vertexShader(NULL),
    m_pixelShader(NULL),
    m_vertexBuffer(NULL),
    m_colorMap(NULL),
    m_colorMapSampler(NULL),
    m_alphaBlendState(NULL)
{
}

//==============================================================================
Spritesheet::~Spritesheet () {

    Reset();

}

//==============================================================================
bool Spritesheet::BuildFromDatafile (const char * filepath) {
    
    Reset();

    CSaruContainer::DataMap                 dataMap;
    CSaruJson::JsonParserCallbackForDataMap callback(dataMap.GetMutator());

    FILE * file;
    fopen_s(&file, filepath, "rt");
    
    CSaruJson::JsonParser parser;
    if (!parser.ParseEntireFile(
        file,
        NULL,
        0,
        &callback
    )) {
        ASSERT(0 && "Failed to parse spritesheet file.");
        fclose(file);
        return false;
    }
    fclose(file);
    file = nullptr;
    
    CSaruContainer::DataMapReader reader = dataMap.GetReader();
    
    reader.ToChild("spritesheet");
    if (!reader.IsValid())
        return false;

    char  tempStr[512];
    WCHAR tempWStr[512];
        
    // Get spritesheet name
    reader.ToChild("name");
    if (!reader.IsValid())
        return false;
    if (!reader.ReadStringSafe(tempStr, arrsize(tempStr)))
        return false;
    m_name.clear();
    swprintf_s(tempWStr, L"%S", tempStr);
    m_name = tempWStr;
    
    // Get image filepath
    reader.PopNode().ToChild("imageFile");
    if (!reader.IsValid()) {
        Reset();
        return false;
    }
    if (!reader.ReadStringSafe(tempStr, arrsize(tempStr)))
        return false;
    m_imageFilepath.clear();
    swprintf_s(tempWStr, L"%S", tempStr);
    m_imageFilepath = tempWStr;
        
    // Prepare to read in animations
    reader.PopNode().ToChild("animations");
    if (!reader.IsValid()) {
        Reset();
        return false;
    }
    
    // Try reading in each animation
    for (reader.ToFirstChild(); reader.IsValid(); reader.ToNextSibling()) {
    
        CSaruContainer::DataMapReader animReader(reader);
        m_animations.push_back(Animation());
        Animation & anim = m_animations.back();
        
        anim.FromDataMap(reader);
    }
    
    // Store source filename
    for (const char * src = filepath; *src; ++src)
        m_sourceFilepath.push_back(*src);
    
    if (!PrepareGraphicsResources())
        return false;
    
    return true;

}

//==============================================================================
const Spritesheet::Animation * Spritesheet::GetAnimation (unsigned index) const {

    if (index >= m_animations.size())
        return NULL;
        
    return &m_animations[index];

}

//==============================================================================
const Spritesheet::Animation * Spritesheet::GetAnimation (const std::wstring & name) const {

    for (const Animation & anim : m_animations) {
        if (anim.name == name)
            return &anim;
    }

    return nullptr;

}

//==============================================================================
unsigned Spritesheet::GetAnimationIndex (const std::wstring & name) const {

    const Spritesheet::Animation * anim = GetAnimation(name);
    if (!anim)
        return unsigned(-1);

    return unsigned(anim - &m_animations[0]);

}

//==============================================================================
bool Spritesheet::PrepareBlendState () {

    D3D11_BLEND_DESC blendDesc;
    SecureZeroMemory(&blendDesc, sizeof(blendDesc));
    blendDesc.RenderTarget[0].BlendEnable           = TRUE;
    blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

    float blend_factor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    HRESULT d3dResult = g_graphicsMgrInternal->GetDevice()->CreateBlendState(&blendDesc, &m_alphaBlendState);
    if (FAILED(d3dResult)) {
        DXTRACE_ERR(
            L"Failed to created blend state for spritesheet.",
            d3dResult
        );
        return false;
    }
    
    g_graphicsMgrInternal->GetContext()->OMSetBlendState(m_alphaBlendState, blend_factor, 0xFFFFFFFF);
    
    return true;

}

//==============================================================================
bool Spritesheet::PrepareConstantBuffers () {

    // Supply initial data
    VertexShaderPerObjectConstantBufferData cbPerObjectData;
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

    ASSERT(!FAILED(d3dResult));
    if (FAILED(d3dResult))
        return false;
    
    return true;

}

//==============================================================================
bool Spritesheet::PrepareGraphicsResources () {

    HRESULT d3dResult;
    
    const wchar_t * spritesheetEffectFilepath = L"Spritesheet.fx";

    m_vertexShader = g_graphicsMgrInternal->LoadVertexShader(
        L"Spritesheet VS",
        spritesheetEffectFilepath,
        "VS_Main"
    );
    m_pixelShader = g_graphicsMgrInternal->LoadPixelShader(
        L"Spritesheet PS",
        spritesheetEffectFilepath,
        "PS_Main"
    );

    // new thing is what?

    d3dResult = D3DX11CreateShaderResourceViewFromFile(
        g_graphicsMgr->GetDevice(),
        m_imageFilepath.c_str(),
        NULL,
        NULL,
        &m_colorMap,
        NULL
    );

    if (FAILED(d3dResult)) {
        DXTRACE_MSG(L"Failed to load the texture image!");
        return false;
    }

    D3D11_SAMPLER_DESC color_map_desc;
    SecureZeroMemory(&color_map_desc, sizeof(color_map_desc));
    color_map_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    color_map_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    color_map_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    color_map_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    //color_map_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    color_map_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    color_map_desc.MaxLOD = D3D11_FLOAT32_MAX;

    d3dResult = g_graphicsMgr->GetDevice()->CreateSamplerState(&color_map_desc, &m_colorMapSampler);

    if (FAILED(d3dResult)) {
        DXTRACE_MSG(L"Failed to create color map sampler state!");
        return false;
    }
    
    if (!PrepareVertexBuffer())
        return false;
        
    if (!PrepareConstantBuffers())
        return false;
    
    if (!PrepareBlendState())
        return false;
    
    return true;

}

//==============================================================================
bool Spritesheet::PrepareVertexBuffer () {

    ID3D11Resource * colorTex;
    m_colorMap->GetResource(&colorTex);
    D3D11_TEXTURE2D_DESC colorTexDesc;
    ( (ID3D11Texture2D*)colorTex )->GetDesc(&colorTexDesc);
    colorTex->Release();
    
    m_textureWidth  = colorTexDesc.Width;
    m_textureHeight = colorTexDesc.Height;

    VertexPos vertices[] = {
        { XMFLOAT3( 0.5f,  0.5f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 0.5f, -0.5f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-0.5f, -0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-0.5f, -0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-0.5f,  0.5f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 0.5f,  0.5f, 1.0f), XMFLOAT2(1.0f, 0.0f) }
    };

    //
    // Create Vertex Buffer
    //

    D3D11_BUFFER_DESC vertex_desc;
    SecureZeroMemory(&vertex_desc, sizeof(vertex_desc));
    vertex_desc.Usage = D3D11_USAGE_DEFAULT;
    vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_desc.ByteWidth = sizeof(VertexPos) * 6;

    D3D11_SUBRESOURCE_DATA resource_data;
    SecureZeroMemory(&resource_data, sizeof(resource_data));
    resource_data.pSysMem = vertices;

    HRESULT d3d_result = g_graphicsMgr->GetDevice()->CreateBuffer(&vertex_desc, &resource_data, &m_vertexBuffer);

    if (FAILED(d3d_result)) {
        DXTRACE_MSG(L"Failed to create vertex buffer!");
        return false;
    }
    
    return true;
}

//==============================================================================
bool Spritesheet::RebuildFromDatafile () {

    std::string temp;
    for (const WCHAR * src = m_sourceFilepath.c_str(); *src; ++src)
        temp.push_back(char(*src));

    return BuildFromDatafile(temp.c_str());

}

//==============================================================================
void Spritesheet::RenderPrep (
    unsigned u,
    unsigned v,
    unsigned width,
    unsigned height
) {

    unsigned int stride = sizeof(VertexPos);
    unsigned int offset = 0;

    ID3D11DeviceContext * d3dContext = g_graphicsMgrInternal->GetContext();
    d3dContext->IASetInputLayout(m_vertexShader->GetInputLayout());
    d3dContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    d3dContext->VSSetShader(m_vertexShader->GetShader(), 0, 0);
    
    d3dContext->PSSetShader(m_pixelShader->GetShader(), 0, 0);
    d3dContext->PSSetShaderResources(0, 1, &m_colorMap);
    d3dContext->PSSetSamplers(0, 1, &m_colorMapSampler);
    
    // Supply data
    VertexShaderPerObjectConstantBufferData cbPerObjectData;
    cbPerObjectData.textureDims.x   = float(m_textureWidth);
    cbPerObjectData.textureDims.y   = float(m_textureHeight);
    cbPerObjectData.frameTexPos.x   = float(u);
    cbPerObjectData.frameTexPos.y   = float(v);
    cbPerObjectData.frameTexDims.x  = float(width);
    cbPerObjectData.frameTexDims.y  = float(height);
    
    D3D11_SUBRESOURCE_DATA dataGlue;
    SecureZeroMemory(&dataGlue, sizeof(dataGlue));
    dataGlue.pSysMem = &cbPerObjectData;
    
    ASSERT(m_perObjectCb);
    d3dContext->UpdateSubresource(
        //g_graphicsMgrInternal->GetRenderResource(),
        m_perObjectCb,
        0,
        NULL,
        &cbPerObjectData,
        0,
        0
    );
    
    d3dContext->VSSetConstantBuffers(0, 1, &m_perObjectCb);

}

//==============================================================================
void Spritesheet::Reset () {

    m_name.clear();
    m_sourceFilepath.clear();
    m_imageFilepath.clear();
    m_animations.clear();
    
    m_vertexShader = NULL;
    m_pixelShader = NULL;
    
    if (m_vertexBuffer)
        m_vertexBuffer->Release();
    m_vertexBuffer = NULL;

    if (m_colorMap)
        m_colorMap->Release();
    m_colorMap = NULL;
    
    if (m_colorMapSampler)
        m_colorMapSampler->Release();
    m_colorMapSampler = NULL;
    
    if (m_alphaBlendState)
        m_alphaBlendState->Release();
    m_alphaBlendState = NULL;

}