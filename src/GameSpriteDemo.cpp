#include "GameSpriteDemo.hpp"
#include "ScratchComponents.h"

static const char * s_spriteFiles[] = {
    "sonic-1-sonic.json",
    "kenney/platformer_redux/spritesheet_players.json",
    "kirby.json",
    "shadow.json",
    //"sonic-1-sonic.json",
};
static const unsigned s_spriteFilesCount = arrsize(s_spriteFiles);

static const char s_levelFile[] = "levels/level0.json";

GameSpriteDemo::GameSpriteDemo(void)
{}


GameSpriteDemo::~GameSpriteDemo(void)
{
  UnloadContent();
}


bool GameSpriteDemo::LoadContent(void)
{

    // Make every GameObject hot-reloadable when 'A' is pressed
    for (unsigned i = 0; i < s_goCount; ++i) {
        m_gameObjects[i].AddComponent(new GocTest());
        m_gameObjects[i].AddComponent(new GocGamepad());
    }

    Vec3 sprite_pos(200.0f, 100.0f, 0.0f);
    for (unsigned i = 0;  i < s_spriteFilesCount && i < s_goCount;  ++i) {
        GocSprite * sprite = new GocSprite();
        m_gameObjects[i].AddComponent(sprite);
        bool success = sprite->BuildFromDatafile(s_spriteFiles[i]);
        ASSERT(success);

        m_gameObjects[i].AddComponent(new GocJumpMan());
        m_gameObjects[i].AddComponent(new GocLeverDashMan());

        m_gameObjects[i].AddComponent(new GocCamera());
        dynamic_cast<GocCamera *>(m_gameObjects[0].GetComponent(GOC_TYPE_CAMERA))->GetCamera().Setup();

        //m_gameObjects[i].AddComponent(new GocDebugLines());

        //////
        //
        {
            Vec3 scale;
            scale.x = scale.y = 1.6f;
            scale.z = 1.6f;
            m_gameObjects[i].GetTransform().SetScale(scale);
        }
        //
        /////

        m_gameObjects[i].GetTransform().SetPosition(sprite_pos);

        sprite_pos.x += 100.0f;
        sprite_pos.y += 50.0f;
    }

    dynamic_cast<GocCamera *>(m_gameObjects[0].GetComponent(GOC_TYPE_CAMERA))->SetAsActiveCamera();


    // -- go3 --
    {
        GameObject & go3 = m_gameObjects[s_goCount - 1];
        go3.GetTransform().SetPosition(Vec3(300.0f, 200.0f, 0.0f));
        //go3.GetTransform().SetScale(XMFLOAT2(3.0f, 3.0f));

        GocLevel * level = new GocLevel();
        go3.AddComponent(level);
        level->LoadLevel(s_levelFile);


        //go3.AddComponent(new GocDebugLines());
    }

    return true;

}


void GameSpriteDemo::UnloadContent(void)
{
        
    g_graphicsMgr->Shutdown();
    
}


void GameSpriteDemo::Update(float dt)
{
    ++m_demoFrame;

    for (unsigned i = 0; i < s_goCount; ++i) {
        m_gameObjects[i].Update(dt);
    }
}


void GameSpriteDemo::Render () {

    g_graphicsMgr->RenderPre();

    for (unsigned i = 0;  i < s_goCount;  ++i)
        m_gameObjects[i].Render();
    
    g_graphicsMgr->RenderPost();

}
