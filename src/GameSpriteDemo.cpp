#include "GameSpriteDemo.hpp"

static const char * s_spriteFiles[] = {
    "sonic-1-sonic.json",
    "shadow.json",
    //"sonic-1-sonic.json",
};
static const unsigned s_spriteFilesCount = sizeof(s_spriteFiles) / sizeof(s_spriteFiles[0]);

GameSpriteDemo::GameSpriteDemo(void)
{
}


GameSpriteDemo::~GameSpriteDemo(void)
{
  UnloadContent();
}


#include "ScratchComponents.h"
bool GameSpriteDemo::LoadContent(void)
{

    for (unsigned i = 0;  i < s_spriteFilesCount;  ++i) {
        GocSprite * sprite = new GocSprite();
        m_gameObjects[i].AddComponent(sprite);
        sprite->BuildFromDatafile(s_spriteFiles[i]);
    }

    m_gameObjects[0].AddComponent(new CompTest());
    m_gameObjects[0].AddComponent(new GocGamepad());
    m_gameObjects[0].AddComponent(new GocLeverDashMan());

    //////
    //
    {
      XMFLOAT2 scale;
      scale.x = scale.y = 1.6f;
      m_gameObjects[0].GetTransform().SetScale(scale);
    }
    //
    /////

    XMFLOAT2 sprite1_pos(200.0f, 100.0f);
    m_gameObjects[0].GetTransform().SetPosition(sprite1_pos);

    //XMFLOAT2 sprite2_pos(400.0f, 100.0f);
    //m_gameObjects[1].GetTransform()->SetPosition(sprite2_pos);

    return true;
}


void GameSpriteDemo::UnloadContent(void)
{
        
    g_graphicsMgr->Shutdown();
    
}


void GameSpriteDemo::Update(float dt)
{
    ++m_demoFrame;

    for (unsigned i = 0; i < s_spriteFilesCount; ++i) {
        m_gameObjects[i].Update(dt);
    }
}


void GameSpriteDemo::Render () {

    if (g_graphicsMgr->GetContext() == NULL)
        return;

    g_graphicsMgr->RenderPre();

    for (unsigned i = 0;  i < s_spriteFilesCount;  ++i)
        m_gameObjects[i].Render();
    
    g_graphicsMgr->RenderPost();

}
