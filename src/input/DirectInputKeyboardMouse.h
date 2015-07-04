/*
Copyright 2013 Christopher Higgins Barrett

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

class XInputKeyboardMouse {

private: // Data

    IDirectInput8 *       m_directInput;
    IDirectInputDevice8 * m_keyboard;
    IDirectInputDevice8 * m_mouse;
    
    unsigned char m_keyboardState[256];
    DIMOUSESTATE2 m_mouseState;
    
    int m_screenWidth;
    int m_screenHeight;
    int m_mouseX;
    int m_mouseY;
    
private: // Helpers

    bool InitializeKeyboard (HWND hWnd);
    bool InitializeMouse (HWND hWnd);

    bool ReadKeyboard ();
    bool ReadMouse ();
    void ProcessInput ();
    
public: // Construction

    XInputKeyboardMouse ();
    virtual ~XInputKeyboardMouse ();
    
public: // Queries

    bool Startup (HINSTANCE hInst, HWND hWnd, int screenWidth, int screenHeight);
    void Shutdown ();
    bool Frame ();

    bool IsEscapePressed ();
    void GetMouseLocation (int * x, int * y);

};

extern XInputKeyboardMouse * g_keyboardMouse;