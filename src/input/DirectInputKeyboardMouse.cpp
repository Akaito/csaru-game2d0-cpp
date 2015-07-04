// This class largely taken from RasterTek's tutorial(s):
// http://www.rastertek.com/dx11tut13.html

#include "DirectInputKeyboardMouse.h"

//==============================================================================
XInputKeyboardMouse::XInputKeyboardMouse () :
    m_directInput(NULL),
    m_keyboard(NULL),
    m_mouse(NULL)
{
}

//==============================================================================
XInputKeyboardMouse::~XInputKeyboardMouse () {
    Shutdown();
}

//==============================================================================
bool XInputKeyboardMouse::Frame () {

    if (!ReadKeyboard())
        return false;
        
    if (!ReadMouse())
        return false;
        
    ProcessInput();
    
    return true;

}

//==============================================================================
void XInputKeyboardMouse::GetMouseLocation (int * x, int * y) {

    ASSERT(x);
    ASSERT(y);
    
    *x = m_mouseX;
    *y = m_mouseY;

}

//==============================================================================
bool XInputKeyboardMouse::InitializeKeyboard (HWND hWnd) {

    // Initialize the direct input interface for the keyboard
    HRESULT result = m_directInput->CreateDevice(
        GUID_SysKeyboard,
        &m_keyboard,
        NULL
    );
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to create keyboard device.");
        return false;
    }
    
    // Set the data format.  In this case it is a keyboard we can use the predefined data format
    result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to set keyboard data format.");
        return false;
    }
    
    // Set the cooperative level of the keyboard to not share with other programs
    // TODO: Use NONEXCLUSIVE (to do so, we'll have to reacquire the device every time it's lost due to a change of focus)
    result = m_keyboard->SetCooperativeLevel(
        hWnd,
        DISCL_FOREGROUND | DISCL_EXCLUSIVE
    );
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to set keyboard's cooperative level.");
        return false;
    }
    
    result = m_keyboard->Acquire();
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to acquire keyboard device.");
        return false;
    }
    
    return true;

}

//==============================================================================
bool XInputKeyboardMouse::InitializeMouse (HWND hWnd) {

    HRESULT result = m_directInput->CreateDevice(
        GUID_SysMouse,
        &m_mouse,
        NULL
    );
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to create mouse device.");
        return false;
    }
    
    // Set data format
    result = m_mouse->SetDataFormat(&c_dfDIMouse2);
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to set data format for mouse.");
        return false;
    }
    
    result = m_mouse->SetCooperativeLevel(
        hWnd,
        DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
    );
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to set cooperative level for mouse.");
        return false;
    }
    
    result = m_mouse->Acquire();
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to acquire mouse device.");
        return false;
    }
    
    return true;

}

//==============================================================================
bool XInputKeyboardMouse::IsEscapePressed () {

    return m_keyboardState[DIK_ESCAPE] & 0x80;

}

//==============================================================================
void XInputKeyboardMouse::ProcessInput () {

    m_mouseX += m_mouseState.lX;
    m_mouseY += m_mouseState.lY;
    
    m_mouseX = max(0, min(m_mouseX, m_screenWidth));
    m_mouseY = max(0, min(m_mouseY, m_screenHeight));

}

//==============================================================================
bool XInputKeyboardMouse::ReadKeyboard () {

    HRESULT result = m_keyboard->GetDeviceState(
        sizeof(m_keyboardState),
        (LPVOID)&m_keyboardState
    );
    if (FAILED(result)) {
        
        if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
            m_keyboard->Acquire();
        else
            return false;
    }
    
    return true;

}

//==============================================================================
bool XInputKeyboardMouse::ReadMouse () {

    HRESULT result = m_mouse->GetDeviceState(
        sizeof(m_mouseState),
        (LPVOID)&m_mouseState
    );
    if (FAILED(result)) {
    
        if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
            m_mouse->Acquire();
        else
            return false;
    }
    
    return true;

}

//==============================================================================
void XInputKeyboardMouse::Shutdown () {

    if (!g_keyboardMouse)
        return;
        
    g_keyboardMouse = NULL;

    if (m_mouse) {
        m_mouse->Unacquire();
        m_mouse->Release();
        m_mouse = NULL;
    }
    
    if (m_keyboard) {
        m_keyboard->Unacquire();
        m_keyboard->Release();
        m_keyboard = NULL;
    }
    
    if (m_directInput) {
        m_directInput->Release();
        m_directInput = NULL;
    }

}

//==============================================================================
bool XInputKeyboardMouse::Startup (
    HINSTANCE hInst,
    HWND hWnd,
    int screenWidth,
    int screenHeight
) {

    if (g_keyboardMouse)
        return true;
    g_keyboardMouse = this;

    HRESULT result;
    
    m_mouseX = m_mouseY = 0;
    
    // Store the screen size which will be used for positioning the mouse cursor
    m_screenWidth  = screenWidth;
    m_screenHeight = screenHeight;
    
    result = DirectInput8Create(
        hInst,
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (void**)&m_directInput,
        NULL
    );
    if (FAILED(result)) {
        ASSERT(!FAILED(result) && "Failed to create DirectInput8 device.");
        return false;
    }
    
    if (!InitializeKeyboard(hWnd))
        return false;
    
    if (!InitializeMouse(hWnd))
        return false;
        
    return true;

}

//==============================================================================
XInputKeyboardMouse * g_keyboardMouse = NULL;