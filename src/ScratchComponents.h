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
#include "graphics/SpriteAnimation.hpp"
#include "graphics/DebugLine.hpp"


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
    SpriteAnimation m_sprite;

public:
    GocSprite () :
        GameObjectComponent()
    {
        m_type = GOC_TYPE_SPRITE;
    }

private:
    void Render () override {

        XMMATRIX projectionFromWorldMtx;
        XMMATRIX worldFromModelMtx;
        g_graphicsMgrInternal->GetProjectionFromWorldMtx(&projectionFromWorldMtx);
        m_owner->GetTransform().GetWorldFromModelMtx(&worldFromModelMtx);

        m_sprite.Render(worldFromModelMtx, projectionFromWorldMtx);

    }

    void Update (float dt) override {

        m_sprite.Update(dt);

    }

public:
    // Commands
    bool BuildFromDatafile (const char * filepath)  {
        Spritesheet * sheet = g_graphicsMgrInternal->LoadSpritesheet(filepath);
        m_sprite.SetSheet(sheet);
        return sheet;
    }
    bool RebuildFromDatafile ()      { return m_sprite.GetSheet()->RebuildFromDatafile(); }
    bool TrySetAnim (const std::wstring & name) {
        unsigned animIndex = m_sprite.GetSheet()->GetAnimationIndex(name);
        if (animIndex < unsigned(-1)) {
            m_sprite.SetAnimIndex(animIndex);
            return true;
        }
        return false;
    }
    void SetFrameIndex (unsigned index) { m_sprite.SetFrameIndex(index); }

    // Queries
    const SpritesheetFrame * GetCurrentFrame () const    { return m_sprite.GetCurrentFrame(); }
    const SpriteAnimation &  GetSprite () const          { return m_sprite; }
    SpriteAnimation &        GetSprite ()                { return m_sprite; }

};

//==============================================================================
class GocDebugLines : public GameObjectComponent {
private:
    DebugLine m_lines[1];

public:
    GocDebugLines () :
        GameObjectComponent()
    {
        m_type = GOC_TYPE_DEBUG_LINES;
    }

private:
    void Render () override {

        XMMATRIX projectionFromWorldMtx;
        XMMATRIX worldFromModelMtx;
        g_graphicsMgrInternal->GetProjectionFromWorldMtx(&projectionFromWorldMtx);
        m_owner->GetTransform().GetWorldFromModelMtx(&worldFromModelMtx);

        m_lines[0].Render(worldFromModelMtx, projectionFromWorldMtx);

    }

    void Update (float dt) override {

        ref(dt);

        /*
        XMFLOAT2 pos = m_owner->GetTransform().GetPosition();
        pos.y -= 10.0f * dt;
        m_owner->GetTransform().SetPosition(pos);
        //*/

    }

public:

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

        GocGamepad * gamepadComp = dynamic_cast<GocGamepad *>(m_owner->GetComponent(GOC_TYPE_GAMEPAD));
        GocSprite *  spriteComp  = dynamic_cast<GocSprite *>(m_owner->GetComponent(GOC_TYPE_SPRITE));
        assert(gamepadComp);
        assert(spriteComp);

        float isx       = gamepadComp->GetAnalogValueAsFloat(XInputGamepad::ANALOG_TYPE_LEFT_STICK_X);
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

        SpriteAnimation & sprite = spriteComp->GetSprite();
        
        // Animation control
        unsigned oldIndex = sprite.GetAnimationIndex();
        spriteComp->TrySetAnim(L"idle");
        // Skidding
        if (fabs(vx) > max_speed * 0.5f && ((vx < 0.0f && isx > 0.0f) || (vx > 0.0f && isx < 0.0f))) {
            spriteComp->TrySetAnim(L"skid");
        }
        else if (oldIndex == 7 && fabs(vx) > 0.05f)
            spriteComp->TrySetAnim(L"skid");
        // Running
        else if (fabs(vx) > max_speed * 0.9f) {
            spriteComp->TrySetAnim(L"run");
        }
        // Walking
        else if (fabs(vx) > max_speed * 0.1f)
            spriteComp->TrySetAnim(L"walk") || spriteComp->TrySetAnim(L"run");
            
        if (sprite.GetAnimationIndex() != oldIndex)
            spriteComp->SetFrameIndex(0);

        {
            XMFLOAT2 pos = m_owner->GetTransform().GetPosition();
            XMFLOAT2 vel = m_owner->GetTransform().GetVelocity();
            pos.x += vel.x;
            pos.y += vel.y;
            
            const SpritesheetFrame * frame = spriteComp->GetCurrentFrame();
            ASSERT(frame);
            
            if (pos.y < frame->height)
                pos.y = frame->height;
                
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
class GocLevel : public GameObjectComponent {

    Level m_level;

    void Update (float dt) override {
        m_level.Update(dt);
    }

    void Render () override {
        XMMATRIX projectionFromWorldMtx;
        g_graphicsMgrInternal->GetProjectionFromWorldMtx(&projectionFromWorldMtx);
        m_level.Render(m_owner->GetTransform(), projectionFromWorldMtx);
    }

public:
    GocLevel () : GameObjectComponent() {
        m_type = GOC_TYPE_LEVEL;
    }

    bool LoadLevel (const char * filepath) {
        return m_level.BuildFromDatafile(filepath);
    }

    void Reload () {
        m_level.Reload();
    }

};


//==============================================================================
class GocTest : public GameObjectComponent {

    void Update (float dt) override {
        ref(dt);

        GocGamepad * gamepad = dynamic_cast<GocGamepad *>(m_owner->GetComponent(GOC_TYPE_GAMEPAD));
        if (!gamepad || !gamepad->AreButtonsPressed(XInputGamepad::BUTTON_FLAG_A))
            return;

        GocSprite * sprite = dynamic_cast<GocSprite *>(m_owner->GetComponent(GOC_TYPE_SPRITE));
        if (sprite)
            sprite->RebuildFromDatafile();

        GocLevel * level = dynamic_cast<GocLevel *>(m_owner->GetComponent(GOC_TYPE_LEVEL));
        if (level)
            level->Reload();

    }

public:
    GocTest () :
        GameObjectComponent()
    {
        m_type = GOC_TYPE_COMP_TEST;
    }

};
