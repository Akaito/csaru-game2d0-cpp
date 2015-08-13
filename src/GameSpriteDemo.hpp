#pragma once

#include "GameObject.h"

class GameSpriteDemo : public Dx11DemoBase
{
 public:
  GameSpriteDemo(void);
  virtual ~GameSpriteDemo(void);
  
  virtual bool LoadContent(void);
  virtual void UnloadContent(void);
  
  virtual void Update(float dt);
  virtual void Render(void);
 
 private:
  static const unsigned s_goCount = 5;
  GameObject m_gameObjects[s_goCount];
};
