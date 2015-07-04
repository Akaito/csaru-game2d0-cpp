#pragma once

#include "Dx11DemoBase.hpp"

class BlankDemo : public Dx11DemoBase
{
 public:
  BlankDemo(void);
  virtual ~BlankDemo(void);
  
  bool LoadContent(void);
  void UnloadContent(void);
  
  void Update(float dt);
  void Render(void);
};
