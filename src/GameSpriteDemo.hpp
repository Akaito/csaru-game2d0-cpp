#pragma once

#include "GameObject.h"
#include "XInputGamepad.h"

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
  GameObject m_gameObjects[2];
};
