#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <memory> // std::auto_ptr<>
#include "Dx11DemoBase.hpp"

//#include "BlankDemo.hpp"
//#include "TriangleDemo.hpp"
//#include "TextureDemo.hpp"
#include "GameSpriteDemo.hpp"
#include "GameTimer.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI wWinMain(HINSTANCE instance, HINSTANCE /*prev_instance*/, LPWSTR /*command_line*/, int command_show)
{
  //
  // Windows window registration/creation
  //
  
  WNDCLASSEX window_class = {0};
  window_class.cbSize = sizeof(window_class);
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = WndProc;
  window_class.hInstance = instance;
  window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  window_class.lpszMenuName = NULL;
  window_class.lpszClassName = L"DX11BookWindowClass";
  
  if (!RegisterClassEx(&window_class))
    return -1;
  
  RECT rc = {0, 0, 640, 480};
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
  
  HWND hwnd = CreateWindowA("DX11BookWindowClass", "Blank Win32 Window", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      rc.right - rc.left, rc.bottom - rc.top,
      NULL, NULL, instance, NULL);
  
  if (!hwnd)
    return -1;
  
  ShowWindow(hwnd, command_show);
  
  std::auto_ptr<Dx11DemoBase> demo(new GameSpriteDemo());
  
  // Demo Initialize
  bool result = demo->Initialize(instance, hwnd);
  
  // Error reporting if there is an issue
  if (result == false)
    return -1;
    
  GameTimer timer;
  timer.Reset();
  
  MSG msg = {0};
  while (msg.message != WM_QUIT)
  {
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      // Update and Draw
      timer.Tick();
      demo->Update(timer.DeltaTime());
      demo->Render();
    }
  }
  
  // Demo Shutdown
  demo->Shutdown();
  
  return static_cast<int>(msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT paint_struct;
  HDC hDC;
  
  switch (message)
  {
    case WM_PAINT:
    {
      hDC = BeginPaint(hwnd, &paint_struct);
      EndPaint(hwnd, &paint_struct);
      return 0;
    }
    
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }
    
    case WM_CHAR:
    {
      if (wParam == VK_ESCAPE)
      {
        PostQuitMessage(0);
        return 0;
      }
      break;
    }
  }
  
  return DefWindowProc(hwnd, message, wParam, lParam);
}
