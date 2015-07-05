#include "Dx11DemoBase.hpp"
#include <D3Dcompiler.h>
#include "input\DirectInputKeyboardMouse.h"

Dx11DemoBase::Dx11DemoBase(void) :
    m_demoFrame(0)
{}


Dx11DemoBase::~Dx11DemoBase(void) {
    Shutdown();
    //ReportLiveObjects();
}


bool Dx11DemoBase::Initialize (HINSTANCE hInstance, HWND hwnd) {

    IGraphicsMgr::Startup(hInstance, hwnd);

    return LoadContent();

}


void Dx11DemoBase::Shutdown(void)
{
    UnloadContent();
}


bool Dx11DemoBase::LoadContent(void)
{
  // Override wtih demo specifics, if any...
  return true;
}


void Dx11DemoBase::UnloadContent(void)
{
  // Override with demo specifics, if any...
}

