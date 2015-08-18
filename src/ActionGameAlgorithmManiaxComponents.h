/*
Most of the interesting code in here is based very directly on code from the Japaneses game programming book
"ActionGame Algorithm Maniax" ISBN978-4-7973-3895-9
I can't read Japanese well enough to know what kind of licensing they might have on their sample code.
*/

#pragma once

unsigned short AGAM_MODULE_ID = 2;

enum EAgamCompId : unsigned short {
    AGAM_COMP_ID_INVALID = 0,
    AGAM_COMP_ID_JUMP_MAN,
    AGAM_COMP_ID_LEVER_DASH_MAN,
    AGAM_COMP_ID_LEVEL_MAN,
};


//==============================================================================
// Based on ActionGame Algorithm Maniax "Jump" chapter.
class GocJumpMan : public GameObjectComponent {
private: // Data
    float m_jumpSpeed;
    bool  m_canJump;
    bool  m_jumping;

    void Update (float dt) {

        ref(dt);

        GocGamepad * gamepad    = dynamic_cast<GocGamepad *>(m_owner->GetComponent(GOC_TYPE_GAMEPAD));
        GocSprite *  spriteComp = dynamic_cast<GocSprite *>(m_owner->GetComponent(GOC_TYPE_SPRITE));
        ASSERT(gamepad);
        ASSERT(spriteComp);

        Vec3 pos = m_owner->GetTransform().GetPosition();
        Vec3 vel = m_owner->GetTransform().GetVelocity();

        if (m_canJump) {
            if (gamepad->AreButtonsPressed(XInputGamepad::BUTTON_FLAG_A)) {
                m_canJump = false;
                m_jumping = true;
                vel.y     = m_jumpSpeed;
                spriteComp->TrySetAnim(L"jump", 0);
            }
        }
        else if (vel.y < 0.0f && !IsJumping()) {
            spriteComp->TrySetAnim(L"fall", 0);
        }
        else if (vel.y < 0.5f && IsJumping()) {
            spriteComp->TrySetAnim(L"jump-crest", 0);
        }

        /*
        const SpritesheetFrame * frame  = spriteComp->GetCurrentFrame();
        if (pos.y <= frame->height) {
            pos.y = frame->height;
            if (vel.y <= 0.0f) {
                vel.y = 0.0f;
                m_canJump = true;
                m_jumping = false;
                //spriteComp->TrySetAnim(L"land", 0);
            }
        }
        //*/

        m_owner->GetTransform().SetPosition(pos);
        m_owner->GetTransform().SetVelocity(vel);

    }

public:
    GocJumpMan () :
        GameObjectComponent(AGAM_MODULE_ID, AGAM_COMP_ID_JUMP_MAN),
        m_jumpSpeed(4.0f),
        m_canJump(false),
        m_jumping(false)
    {}

    bool CanJump () const   { return m_canJump; }
    bool IsJumping () const { return m_jumping; }
    bool IsFalling () const { return m_owner->GetTransform().GetVelocity().y < 0.0f; }

    void EnableJump () {
        m_canJump = true;
        m_jumping = false;
    }
};


//==============================================================================
// Based on ActionGame Algorithm Maniax "Lever Dash Man" chapter.
class GocLeverDashMan : public GameObjectComponent {

    void Update (float dt) override {

        {
            Vec3 vel = m_owner->GetTransform().GetVelocity();
            //vel.y -= 0.98f * dt;
            vel.y -= 6.0f * dt;
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
        
        Vec3 scale = m_owner->GetTransform().GetScale();
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
        unsigned  oldIndex = sprite.GetAnimationIndex();
        GocJumpMan * jumpComp = dynamic_cast<GocJumpMan * >(m_owner->GetComponent(AGAM_MODULE_ID << 16 | AGAM_COMP_ID_JUMP_MAN));
        if (!jumpComp || jumpComp->CanJump()) {
            // Skidding
            if (fabs(vx) > max_speed * 0.5f && ((vx < 0.0f && isx > 0.0f) || (vx > 0.0f && isx < 0.0f))) {
                spriteComp->TrySetAnim(L"skid", 0);
            }
            else if (oldIndex == 7 && fabs(vx) > 0.05f)
                spriteComp->TrySetAnim(L"skid", 0);
            // Running
            else if (fabs(vx) > max_speed * 0.9f) {
                spriteComp->TrySetAnim(L"run", 0) || spriteComp->TrySetAnim(L"walk", 0);
            }
            // Walking
            else if (fabs(vx) > max_speed * 0.1f) {
                spriteComp->TrySetAnim(L"walk", 0) || spriteComp->TrySetAnim(L"run", 0);
            }
            else
                spriteComp->TrySetAnim(L"idle", 0) || spriteComp->TrySetAnim(L"stand", 0);
        }

        {
            Vec3 pos = m_owner->GetTransform().GetPosition();
            Vec3 vel = m_owner->GetTransform().GetVelocity();
            pos.x += vel.x;
            pos.y += vel.y;
            
            const SpritesheetFrame * frame = spriteComp->GetCurrentFrame();
            ASSERT(frame);
            
            /*
            if (pos.y < frame->height) {
                pos.y = frame->height;
                vel.y = 0.0f;
            }
            //*/
                
            m_owner->GetTransform().SetPosition(pos);
            m_owner->GetTransform().SetVelocity(vel);
        }

    }

public:
    GocLeverDashMan () : GameObjectComponent(AGAM_MODULE_ID << 16 | AGAM_COMP_ID_LEVER_DASH_MAN)
    {}

};


//==============================================================================
class GocLevelMan : public GameObjectComponent {

    GameObject * m_levelGob;

    void Update (float dt) override {

        ref(dt);

        GocJumpMan *  jumpGoc  = dynamic_cast<GocJumpMan *>(m_owner->GetComponent(AGAM_MODULE_ID << 16 | AGAM_COMP_ID_JUMP_MAN));
        GocLevel *    levelGoc = dynamic_cast<GocLevel *>(m_levelGob->GetComponent(GOC_TYPE_LEVEL));
        const Level * level    = levelGoc->GetLevel();

        Vec3 pos                   = m_owner->GetTransform().GetPosition();
        Vec3 vel                   = m_owner->GetTransform().GetVelocity();
        const float halfTileWidth  = level->GetTileWidth()  * m_levelGob->GetTransform().GetScale().x * 0.5f;
        const float halfTileHeight = level->GetTileHeight() * m_levelGob->GetTransform().GetScale().y * 0.5f;

        Vec3 levelMin, levelMax;
        levelGoc->GetExtents(&levelMin, &levelMax);
        pos.ClampThis(levelMin, levelMax);

        unsigned charTileX, charTileY;
        levelGoc->GetTilePosFromWorldPos(pos, &charTileX, &charTileY);

        Vec3 tilePos;
        // check underneath
        const Level::TileData * tileData = level->GetTileData(charTileX, charTileY-1);
        levelGoc->GetTilePosInWorld(charTileX-1, charTileY, &tilePos);
        if (tilePos.y < pos.y && vel.y <= 0.0f) {
            pos.y = tilePos.y + halfTileHeight;
            vel.y = 0.0f;
            jumpGoc->EnableJump();
        }

        /*
        unsigned i       = 0;
        Vec3     tilePos;
        while (const Level::TileData * tileData = level->EnumTileData(&i)) {
            if (tileData->collision == Level::ETileCollision::None)
                continue;

            levelGoc->GetTilePosInWorld(i-1, &tilePos);
            if (pos.x < tilePos.x - halfTileWidth || tilePos.x + halfTileWidth < pos.x)
                continue;
            if (pos.y < tilePos.y - halfTileHeight || tilePos.y + halfTileHeight < pos.y)
                continue;

            if (tilePos.x < pos.x && vel.x < 0.0f) {
                pos.x = tilePos.x + halfTileWidth;
                vel.x = 0.0f;
            }
            else if (pos.x < tilePos.x && 0.0f < vel.x) {
                pos.x = tilePos.x - halfTileWidth;
                vel.x = 0.0f;
            }

            if (tilePos.y < pos.y && vel.y <= 0.0f) {
                pos.y = tilePos.y + halfTileHeight;
                vel.y = 0.f;
                jumpGoc->EnableJump();
            }
            else if (pos.y < tilePos.y && 0.0f < vel.y) {
                pos.y = tilePos.y - halfTileHeight;
                vel.y = 0.0f;
            }

            break;
        }
        //*/

        m_owner->GetTransform().SetPosition(pos);
        m_owner->GetTransform().SetVelocity(vel);

    }

public:
    GocLevelMan (GameObject * levelGob) :
        GameObjectComponent(AGAM_MODULE_ID << 16 | AGAM_COMP_ID_LEVEL_MAN),
        m_levelGob(levelGob)
    {}
};
