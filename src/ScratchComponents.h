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

#include "GameObjectComponent.h"


//==============================================================================
class GocGamepad : public GameObjectComponent {
protected:
    XInputGamepad m_gamepad;

    void Update (float dt) override {
        ref(dt);
        m_gamepad.Update();
    }

public:
    GocGamepad () :
        GameObjectComponent()
    {
        m_type = GOC_TYPE_GAMEPAD;
    }

    bool  IsConnected () const                                                 { return m_gamepad.IsConnected(); }
    bool  AreButtonsPressed (XInputGamepad::EButtonFlags buttonFlags) const    { return m_gamepad.AreButtonsPressed(buttonFlags); }
    float GetAnalogValueAsFloat (XInputGamepad::EAnalogTypes analogType) const { return m_gamepad.GetAnalogValueAsFloat(analogType); }
    float GetLeftStickXAsFloat () const                                        { return m_gamepad.GetLeftStickXAsFloat(); }
    //float GetLeftStickYAsFloat () const;
    //float GetLeftTriggerAsFloat () const;
    //float GetRightStickXAsFloat () const;
    //float GetRightStickYAsFloat () const;
    //float GetRightTriggerAsFloat () const;
};


//==============================================================================
class GocSprite : public GameObjectComponent {
private:
    Spritesheet * m_sheet2;
    unsigned      m_animIndex;
    unsigned      m_frameIndex;
    float         m_timeOnFrameSeconds;

public:
    GocSprite () :
        GameObjectComponent(),
        m_sheet2(nullptr),
        m_animIndex(0),
        m_frameIndex(0),
        m_timeOnFrameSeconds(0)
    {
        m_type = GOC_TYPE_SPRITE;
    }

private:
    void Render () override {

        const Spritesheet::Frame & frame = m_sheet2->GetAnimation(m_animIndex)->frames[m_frameIndex];
        
        XMMATRIX view_projection;
        g_graphicsMgrInternal->GetViewProjectionMatrix(&view_projection);

        XMMATRIX world = m_owner->GetTransform().GetWorldMatrix();
        XMMATRIX mvp = XMMatrixMultiply(world, view_projection);
        mvp = XMMatrixTranspose(mvp);

        ID3D11Buffer *        buffer     = g_graphicsMgrInternal->GetRenderResource();
        ID3D11DeviceContext * d3dContext = g_graphicsMgrInternal->GetContext();

        m_sheet2->RenderPrep(
            frame.x,
            frame.y,
            frame.width,
            frame.height
        );
        
        float scale[2];
        scale[0] = frame.width * 10.0f;
        scale[1] = frame.height * 10.0f;
        
        d3dContext->UpdateSubresource(buffer, 0, NULL, &mvp, 0, 0);
        d3dContext->VSSetConstantBuffers(1, 1, &buffer);

        d3dContext->Draw(6, 0);



        /*
        m_sheet2->RenderPrep(
            frame.x,
            frame.y,
            frame.width,
            frame.height
        );
        
        scale[0] = frame.width * 10.0f;
        scale[1] = frame.height * 15.0f;
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

    void Update (float dt) override {

        m_timeOnFrameSeconds += dt;

        // Animations
        const Spritesheet::Animation * anim  = m_sheet2->GetAnimation(m_animIndex);
        const Spritesheet::Frame *     frame = anim ? &anim->frames[m_frameIndex] : NULL;
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

public:
    bool                           BuildFromDatafile (const char * filepath)  {
        m_sheet2 = g_graphicsMgrInternal->LoadSpritesheet(filepath);
        return m_sheet2;
    }
    bool                           RebuildFromDatafile ()                     { return m_sheet2->RebuildFromDatafile();       }
    const Spritesheet::Animation * GetAnimation (unsigned index) const        { return m_sheet2->GetAnimation(index);         }

    unsigned GetAnimIndex () const         { return m_animIndex;  }
    void     SetAnimIndex (unsigned index) { m_animIndex = index; }
    bool     TrySetAnim (const std::wstring & name) {
        unsigned animIndex = m_sheet2->GetAnimationIndex(name);
        if (animIndex < unsigned(-1)) {
            SetAnimIndex(animIndex);
            return true;
        }
        return false;
    }

    unsigned GetFrameIndex () const         { return m_frameIndex;  }
    void     SetFrameIndex (unsigned index) { m_frameIndex = index; }

};


//==============================================================================
class GocLeverDashMan : public GameObjectComponent {

    void Update (float dt) override {

        {
            XMFLOAT2 vel = m_owner->GetTransform().GetVelocity();
            vel.y -= 0.98f * dt;
            m_owner->GetTransform().SetVelocity(vel);
        }

        ref(dt);

        GocGamepad * gamepad = dynamic_cast<GocGamepad *>(m_owner->GetComponent(GOC_TYPE_GAMEPAD));
        GocSprite *  sprite  = dynamic_cast<GocSprite *>(m_owner->GetComponent(GOC_TYPE_SPRITE));
        assert(gamepad);
        assert(sprite);

        float isx       = gamepad->GetAnalogValueAsFloat(XInputGamepad::ANALOG_TYPE_LEFT_STICK_X);
        float vx        = m_owner->GetTransform().GetVelocity().x;
        float max_speed = 0.5f  * 10.0f;
        float accel     = 0.01f * 10.0f;
        
        if (isx > 0.0f)
            vx += accel;
        else if (isx < 0.0f)
            vx -= accel;
        else {
            if (vx > 0.0f)
                vx -= accel * 0.5f;
            else if (vx < 0.0f)
                vx += accel * 0.5f;
        }
            
        if (fabs(vx) < 0.001f)
            vx = 0.0f;
            
        vx = max(-max_speed, min(vx, max_speed));
            
        float angle = vx / max_speed * 0.1f;
        
        
        m_owner->GetTransform().SetVelocity(vx, m_owner->GetTransform().GetVelocity().y);
        
        XMFLOAT2 scale = m_owner->GetTransform().GetScale();
        if (vx < 0.0f && scale.x > 0.0f)
            scale.x *= -1.0f;
        else if (vx > 0.0f && scale.x < 0.0f)
            scale.x *= -1.0f;
        //if (vx > 0.0f) scale.y = 2.0f;
        //else scale.y = 1.6f;
        m_owner->GetTransform().SetScale(scale);
        
        m_owner->GetTransform().SetRotation(angle);
        
        // Animation control
        unsigned oldIndex = sprite->GetAnimIndex();
        sprite->TrySetAnim(L"idle");
        // Skidding
        if (fabs(vx) > max_speed * 0.5f && ((vx < 0.0f && isx > 0.0f) || (vx > 0.0f && isx < 0.0f))) {
            sprite->TrySetAnim(L"skid");
        }
        else if (oldIndex == 7 && fabs(vx) > 0.05f)
            sprite->TrySetAnim(L"skid");
        // Running
        else if (fabs(vx) > max_speed * 0.9f) {
            sprite->TrySetAnim(L"run");
        }
        // Walking
        else if (fabs(vx) > max_speed * 0.1f)
            sprite->TrySetAnim(L"walk") || sprite->TrySetAnim(L"run");
            
        if (sprite->GetAnimIndex() != oldIndex)
            sprite->SetFrameIndex(0);

        {
            XMFLOAT2 pos = m_owner->GetTransform().GetPosition();
            XMFLOAT2 vel = m_owner->GetTransform().GetVelocity();
            pos.x += vel.x;
            pos.y += vel.y;
            
            const Spritesheet::Frame & frame = sprite->GetAnimation(sprite->GetAnimIndex())->frames[sprite->GetFrameIndex()];
            
            if (pos.y < frame.height)
                pos.y = frame.height;
                
            m_owner->GetTransform().SetPosition(pos);
        }

    }

public:
    GocLeverDashMan () :
        GameObjectComponent()
    {
        m_type = GOC_TYPE_LEVER_DASH_MAN;
    }

};


//==============================================================================
class CompTest : public GameObjectComponent {

    void Update (float dt) override {
        ref(dt);
        int i = 4;
        i = 5;

        GocGamepad * gamepad = dynamic_cast<GocGamepad *>(m_owner->GetComponent(GOC_TYPE_GAMEPAD));
        GocSprite *  sprite  = dynamic_cast<GocSprite *>(m_owner->GetComponent(GOC_TYPE_SPRITE));
        assert(gamepad);
        assert(sprite);
            
        if (gamepad->AreButtonsPressed(XInputGamepad::BUTTON_FLAG_A))
            sprite->RebuildFromDatafile();

    }

public:
    CompTest () :
        GameObjectComponent()
    {
        m_type = GOC_TYPE_COMP_TEST;
    }

};
