#include "GameSpriteDemo.hpp"
#include "ScratchComponents.h"

static const char * s_spriteFiles[] = {
    "sonic-1-sonic.json",
    "shadow.json",
    //"sotn-debug-terrain.json",
    //"sonic-1-sonic.json",
};
static const unsigned s_spriteFilesCount = arrsize(s_spriteFiles);

static const char s_levelFile[] = "levels/level0.json";

GameSpriteDemo::GameSpriteDemo(void)
{
}


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

    for (unsigned i = 0;  i < s_spriteFilesCount && i < s_goCount;  ++i) {
        GocSprite * sprite = new GocSprite();
        m_gameObjects[i].AddComponent(sprite);
        bool success = sprite->BuildFromDatafile(s_spriteFiles[i]);
        ASSERT(success);
    }

    m_gameObjects[0].AddComponent(new GocLeverDashMan());
    m_gameObjects[1].AddComponent(new GocLeverDashMan());

    m_gameObjects[0].AddComponent(new GocDebugLines());

    //////
    //
    {
        XMFLOAT2 scale;
        scale.x = scale.y = 1.6f;
        m_gameObjects[0].GetTransform().SetScale(scale);
        m_gameObjects[1].GetTransform().SetScale(scale);
    }
    //
    /////

    XMFLOAT2 sprite1_pos(200.0f, 100.0f);
    m_gameObjects[0].GetTransform().SetPosition(sprite1_pos);

    XMFLOAT2 sprite2_pos(400.0f, 100.0f);
    m_gameObjects[1].GetTransform().SetPosition(sprite2_pos);


    // -- go3 --
    {
        GameObject & go3 = m_gameObjects[2];
        go3.GetTransform().SetPosition(XMFLOAT2(300.0f, 200.0f));
        go3.GetTransform().SetScale(XMFLOAT2(3.0f, 3.0f));

        GocLevel * level = new GocLevel();
        go3.AddComponent(level);
        level->LoadLevel(s_levelFile);


        go3.AddComponent(new GocDebugLines());
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

    if (g_graphicsMgr->GetContext() == NULL)
        return;

    g_graphicsMgr->RenderPre();

    for (unsigned i = 0;  i < s_goCount;  ++i)
        m_gameObjects[i].Render();
    
    g_graphicsMgr->RenderPost();

}
