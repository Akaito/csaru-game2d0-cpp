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

#include "GameObjectComponent.h"
#include <SpriteAnimation.h>
#include <Spritesheet.h>
#include <Camera.h>
#include <XInputGamepad.h>
//#include "graphics/DebugLine.hpp"

#include "Levels\Level.hpp"


enum EScratchGocType : unsigned {
    GOC_TYPE_INVALID     = 0,
    GOC_TYPE_COMP_TEST   = 1 << 16 | 1,
    GOC_TYPE_GAMEPAD     = 1 << 16 | 2,
    GOC_TYPE_SPRITE      = 1 << 16 | 3,
    GOC_TYPE_DEBUG_LINES = 1 << 16 | 4,
    GOC_TYPE_LEVEL       = 1 << 16 | 5,
    GOC_TYPE_CAMERA      = 1 << 16 | 6,
};

//==============================================================================
class GocCamera : public GameObjectComponent {

    Camera m_camera;

    void Update (float dt) override {
        ref(dt);
        Vec3 pos = m_camera.GetPosition();
        pos = m_owner->GetTransform().GetPosition();
        //pos.x = m_owner->GetTransform().GetPosition().x;
        m_camera.SetPosition(pos);
    }

    void Render () override {
    }

public:
    GocCamera () : GameObjectComponent(GOC_TYPE_CAMERA)
    {}

    Camera & GetCamera () { return m_camera; }

    void SetAsActiveCamera () {
        g_graphicsMgr->SetActiveCamera(&m_camera);
    }

};


//==============================================================================
class GocGamepad : public GameObjectComponent {
protected:
    XInputGamepad m_gamepad;

    void Update (float dt) override {
        ref(dt);
        m_gamepad.Update();
    }

public:
    GocGamepad () : GameObjectComponent(GOC_TYPE_GAMEPAD)
    {}

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
    GocSprite () : GameObjectComponent(GOC_TYPE_SPRITE)
    {}

private:
    void Render () override {

        Mtx44 worldFromModelMtx;
        m_owner->GetTransform().GetWorldFromModelMtx(&worldFromModelMtx);

        m_sprite.Render(worldFromModelMtx);

    }

    void Update (float dt) override {
        m_sprite.Update(dt);
    }

public:
    // Commands
    bool BuildFromDatafile (const char * filepath)  {
        Spritesheet * sheet = g_graphicsMgr->LoadSpritesheet(filepath);
        m_sprite.SetSheet(sheet);
        return sheet;
    }

    bool RebuildFromDatafile ()      { return m_sprite.GetSheet()->RebuildFromDatafile(); }

    bool TrySetAnim (const std::wstring & name) {
        unsigned animIndex = m_sprite.GetSheet()->GetAnimationIndex(name);
        if (animIndex >= unsigned(-1))
            return false;

        m_sprite.SetAnimIndex(animIndex);
        return true;
    }

    bool TrySetAnim (const std::wstring & name, unsigned frameIndexIfAnimChanges) {
        const unsigned oldIndex = m_sprite.GetAnimationIndex();
        if (!TrySetAnim(name))
            return false;

        if (oldIndex != m_sprite.GetAnimationIndex())
            SetFrameIndex(frameIndexIfAnimChanges);

        return true;
    }

    void SetFrameIndex (unsigned index) {
        m_sprite.SetFrameIndex(index);
        m_sprite.SetTimeOnFrameSeconds(0.0f);
    }

    // Queries
    const SpritesheetFrame * GetCurrentFrame () const    { return m_sprite.GetCurrentFrame(); }
    const SpriteAnimation &  GetSprite () const          { return m_sprite; }
    SpriteAnimation &        GetSprite ()                { return m_sprite; }

};


//==============================================================================
class GocDebugLines : public GameObjectComponent {
public:
    GocDebugLines () : GameObjectComponent(GOC_TYPE_DEBUG_LINES)
    {}

private:
    void Update (float dt) override {

        ref(dt);

        g_graphicsMgr->DebugDrawLine(
            Vec3(0.0f, 0.0f, 0.0f),
            m_owner->GetTransform().GetPosition(),
            Vec3(1.0f, 0.0f, 1.0f),
            Vec3(1.0f, 1.0f, 0.0f)
        );

    }

};


//==============================================================================
class GocLevel : public GameObjectComponent {

    Level m_level;

    void Update (float dt) override {
        m_level.Update(dt);
    }

    void Render () override {
        m_level.Render(m_owner->GetTransform());
    }

public:
    GocLevel () : GameObjectComponent(GOC_TYPE_LEVEL)
    {}

    bool LoadLevel (const char * filepath) {
        return m_level.BuildFromDatafile(filepath);
    }

    const Level * GetLevel () const { return &m_level; }
    bool          GetTilePosInWorld (unsigned tileIndex, Vec3 * posOut) const {
        posOut->z = 0.5f;

        unsigned tileX, tileY;
        if (!m_level.GetTilePos(tileIndex, &tileX, &tileY))
            return false;

        posOut->x = m_level.GetTileWidth() * tileX * m_owner->GetTransform().GetScale().x;
        posOut->y = m_level.GetTileWidth() * tileY * m_owner->GetTransform().GetScale().y;

        posOut->x += m_owner->GetTransform().GetPosition().x;
        posOut->y += m_owner->GetTransform().GetPosition().y;
        return true;
    }
    bool          GetTilePosInWorld (unsigned tileX, unsigned tileY, Vec3 * posOut) const {
        return GetTilePosInWorld(tileY * m_level.GetWidthInTiles() + tileX, posOut);
    }

    void GetTileSize (Vec3 * dimsOut) const {
        ASSERT(dimsOut);
        dimsOut->x = m_level.GetTileWidth() * m_owner->GetTransform().GetScale().x;
        dimsOut->y = m_level.GetTileHeight() * m_owner->GetTransform().GetScale().y;
        dimsOut->z = 1.0f;
    }

    bool GetTilePosFromWorldPos (const Vec3 & worldPos, unsigned * tileXOut, unsigned * tileYOut) {
        ASSERT(tileXOut);
        ASSERT(tileYOut);
        Vec3 thisMin, thisMax;
        GetExtents(&thisMin, &thisMax);
        if (!worldPos.ContainedBy(thisMin, thisMax))
            return false;

        float localX = worldPos.x - thisMin.x;
        float localY = worldPos.y - thisMin.y;

        Vec3 tileSize;
        GetTileSize(&tileSize);
        *tileXOut = unsigned(localX / tileSize.x);
        *tileYOut = unsigned(localY / tileSize.y);
        return true;
    }

    void GetExtents (Vec3 * minOut, Vec3 * maxOut) const {
        ASSERT(minOut);
        ASSERT(maxOut);
        GetTilePosInWorld(0, minOut);
        GetTilePosInWorld((m_level.GetWidthInTiles() * m_level.GetHeightInTiles()) - 1, maxOut);
        minOut->x *= m_owner->GetTransform().GetScale().x;
        maxOut->y *= m_owner->GetTransform().GetScale().y;

        Vec3 tileDims;
        GetTileSize(&tileDims);
        tileDims *= 0.5f;
        *minOut -= tileDims;
        *maxOut += tileDims;
        minOut->z = 0.0f;
        maxOut->z = 1.0f;
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
        if (!gamepad || !gamepad->AreButtonsPressed(XInputGamepad::BUTTON_FLAG_START))
            return;

        GocSprite * sprite = dynamic_cast<GocSprite *>(m_owner->GetComponent(GOC_TYPE_SPRITE));
        if (sprite)
            sprite->RebuildFromDatafile();

        GocLevel * level = dynamic_cast<GocLevel *>(m_owner->GetComponent(GOC_TYPE_LEVEL));
        if (level)
            level->Reload();

    }

public:
    GocTest () : GameObjectComponent(GOC_TYPE_COMP_TEST)
    {}

};

// Hacks!!
#include "ActionGameAlgorithmManiaxComponents.h"
