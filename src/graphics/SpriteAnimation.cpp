#include "SpriteAnimation.hpp"
#include "Spritesheet.h"

//==============================================================================
const SpritesheetFrame * SpriteAnimation::GetCurrentFrame () const {

    if (!m_spritesheet)
        return nullptr;

    const Spritesheet::Animation * anim = m_spritesheet->GetAnimation(m_animIndex);
    if (!anim || m_frameIndex >= anim->frames.size())
        return nullptr;

    return &anim->frames[m_frameIndex];

}

//==============================================================================
void SpriteAnimation::Render (const XMMATRIX & world, const XMMATRIX & viewProjection) {

    ASSERT(m_spritesheet);
    
    const SpritesheetFrame & frame = m_spritesheet->GetAnimation(m_animIndex)->frames[m_frameIndex];
    m_spritesheet->RenderPrep(
        frame.x,
        frame.y,
        frame.width,
        frame.height
    );
    
    XMMATRIX mvp = XMMatrixMultiply(world, viewProjection);
    mvp = XMMatrixTranspose(mvp);
    
    ID3D11Buffer *        buffer     = g_graphicsMgrInternal->GetRenderResource();
    ID3D11DeviceContext * d3dContext = g_graphicsMgrInternal->GetContext();
    d3dContext->UpdateSubresource(buffer, 0, nullptr, &mvp, 0, 0);
    d3dContext->VSSetConstantBuffers(1, 1, &buffer);

    d3dContext->Draw(6, 0);



    /*
    m_sheet2->RenderPrep(
        frame.x,
        frame.y,
        frame.width,
        frame.height
    );
    
    world = m_owner->GetTransform().GetWorldMatrix();
    XMMATRIX test = XMMatrixTranslation(50.0f, 0.0f, 0.0f);
    mvp = XMMatrixMultiply(world, test);
    mvp = XMMatrixMultiply(mvp, view_projection);
    mvp = XMMatrixTranspose(mvp);
    
    d3dContext->UpdateSubresource(buffer, 0, NULL, &mvp, 0, 0);
    d3dContext->VSSetConstantBuffers(1, 1, &buffer);

    d3dContext->Draw(6, 0);
    //*/

}

//==============================================================================
void SpriteAnimation::SetAnimIndex (unsigned index) {
    m_animIndex = index;
}

//==============================================================================
void SpriteAnimation::SetFrameIndex (unsigned index) {
    m_frameIndex = index;
}

//==============================================================================
void SpriteAnimation::SetSheet (Spritesheet * sheet) {

    m_spritesheet        = sheet;
    m_animIndex          = 0;
    m_frameIndex         = 0;
    m_timeOnFrameSeconds = 0.0f;

}

//==============================================================================
void SpriteAnimation::Update (float dt) {

    if (!m_spritesheet)
        return;

    m_timeOnFrameSeconds += dt;

    // Animations
    const Spritesheet::Animation * anim  = m_spritesheet->GetAnimation(m_animIndex);
    const SpritesheetFrame *       frame = anim ? &anim->frames[m_frameIndex] : NULL;
    if (!frame)
        return;

    float frameDurationSeconds = float(frame->durationMs) / 1000.0f;

    while (m_timeOnFrameSeconds > frameDurationSeconds && frame->durationMs) {
        
        m_timeOnFrameSeconds -= frameDurationSeconds;
        
        ++m_frameIndex;
        m_frameIndex %= anim->frames.size();
        frame = &anim->frames[m_frameIndex];
    }

}