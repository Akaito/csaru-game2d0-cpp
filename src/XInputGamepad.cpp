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

#include "XInputGamepad.h"

//==============================================================================
// Constants
//==============================================================================
static const unsigned s_disconnectedCheckDelay = 3 * 60; // About one every three seconds

//==============================================================================
XInputGamepad::XInputGamepad ()
    : m_controllerIndex(0),
      m_isConnected(false),
      m_lastPacketNumber(0)
{
    ZeroStateBuffers();
}

//==============================================================================
XInputGamepad::~XInputGamepad () {
}

//==============================================================================
void XInputGamepad::AdvanceStateBuffers () {

    for (int i = BUFFER_SLOTS - 1; i >= 0; --i) {
        memcpy(
            &m_stateBuffer[BUFFER_SLOT_PREVIOUS],
            &m_stateBuffer[BUFFER_SLOT_CURRENT],
            sizeof(m_stateBuffer[0])
        );
    }
    
    SecureZeroMemory(&m_stateBuffer[BUFFER_SLOT_CURRENT], sizeof(m_stateBuffer[0]));

}

//==============================================================================
bool XInputGamepad::AreButtonsPressed (EButtonFlags buttonFlags) const {

    if (!m_isConnected)
        return false;
        
    return (m_stateBuffer[BUFFER_SLOT_CURRENT].buttons & buttonFlags) ? true : false;

}

//==============================================================================
float XInputGamepad::GetAnalogValueAsFloat (EAnalogTypes analogType) const {

    switch (analogType) {
        case ANALOG_TYPE_LEFT_STICK_X:  return GetLeftStickXAsFloat();
        case ANALOG_TYPE_LEFT_STICK_Y:  return GetLeftStickYAsFloat();
        case ANALOG_TYPE_RIGHT_STICK_X: return GetRightStickXAsFloat();
        case ANALOG_TYPE_RIGHT_STICK_Y: return GetRightStickYAsFloat();
        case ANALOG_TYPE_LEFT_TRIGGER:  return GetLeftTriggerAsFloat();
        case ANALOG_TYPE_RIGHT_TRIGGER: return GetRightTriggerAsFloat();
    }
    
    assert(0 && "Bad value given to XInputGamepad::GetAnalogValueAsFloat().");
    return 0.0f;

}

//==============================================================================
float XInputGamepad::GetLeftStickXAsFloat () const {

    if (!m_isConnected)
        return 0.0f;

    short valueRaw = m_stateBuffer[BUFFER_SLOT_CURRENT].leftStickX;
    if (abs(valueRaw) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        return 0.0f;

    return (float)valueRaw / MAX_SIGNED_SHORT;

}

//==============================================================================
float XInputGamepad::GetLeftStickYAsFloat () const {

    if (!m_isConnected)
        return 0.0f;

    short valueRaw = m_stateBuffer[BUFFER_SLOT_CURRENT].leftStickY;
    if (abs(valueRaw) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        return 0.0f;

    return (float)valueRaw / MAX_SIGNED_SHORT;

}

//==============================================================================
float XInputGamepad::GetLeftTriggerAsFloat () const {

    if (!m_isConnected)
        return 0.0f;

    unsigned char valueRaw = m_stateBuffer[BUFFER_SLOT_CURRENT].leftTrigger;
    if (valueRaw < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
        return 0.0f;

    return (float)valueRaw / MAX_UNSIGNED_CHAR;

}

//==============================================================================
float XInputGamepad::GetRightStickXAsFloat () const {

    if (!m_isConnected)
        return 0.0f;

    short valueRaw = m_stateBuffer[BUFFER_SLOT_CURRENT].rightStickX;
    if (abs(valueRaw) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
        return 0.0f;

    return (float)valueRaw / MAX_SIGNED_SHORT;

}

//==============================================================================
float XInputGamepad::GetRightStickYAsFloat () const {

    if (!m_isConnected)
        return 0.0f;

    short valueRaw = m_stateBuffer[BUFFER_SLOT_CURRENT].rightStickY;
    if (abs(valueRaw) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
        return 0.0f;

    return (float)valueRaw / MAX_SIGNED_SHORT;

}

//==============================================================================
float XInputGamepad::GetRightTriggerAsFloat () const {

    if (!m_isConnected)
        return 0.0f;

    unsigned char valueRaw = m_stateBuffer[BUFFER_SLOT_CURRENT].rightTrigger;
    if (valueRaw < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
        return 0.0f;

    return (float)valueRaw / MAX_UNSIGNED_CHAR;

}

//==============================================================================
void XInputGamepad::OnConnect () {

    m_isConnected      = true;
    m_lastPacketNumber = 0;

}

//==============================================================================
void XInputGamepad::OnDisconnect () {

    m_isConnected      = false;
    m_lastPacketNumber = 0; // Doubles as recheck delay counter for disconnected controllers
    ZeroStateBuffers();

}

//==============================================================================
void XInputGamepad::Update () {

    // If this controller isn't connected, don't check on it too frequently
    if (!m_isConnected && ++m_lastPacketNumber < s_disconnectedCheckDelay) {
        return;
    }

    // Get current xInputState
    XINPUT_STATE xInputState;
    SecureZeroMemory(&xInputState, sizeof(xInputState));
    
    // Get the xInputState of the controller from XInput
    unsigned result = XInputGetState(m_controllerIndex, &xInputState);
    if (result != ERROR_SUCCESS) {
    
        if (m_isConnected)
            OnDisconnect();
        return;
    }
    
    if (!m_isConnected)
        OnConnect();
    
    // If the controller hasn't changed xInputState, take no further action
    if (xInputState.dwPacketNumber == m_lastPacketNumber)
        return;

    AdvanceStateBuffers();
    m_lastPacketNumber = xInputState.dwPacketNumber;
    
    // Update most recent xInputState from XInput xInputState
    ControllerState & state = m_stateBuffer[BUFFER_SLOT_CURRENT];
    // Copy button state
    memcpy(
        &state.buttons,
        &xInputState.Gamepad.wButtons,
        min(sizeof(state.buttons), sizeof(xInputState.Gamepad.wButtons))
    );
    // Copy analog stick and trigger states
    state.leftStickX   = xInputState.Gamepad.sThumbLX;
    state.leftStickY   = xInputState.Gamepad.sThumbLY;
    state.rightStickX  = xInputState.Gamepad.sThumbRX;
    state.rightStickY  = xInputState.Gamepad.sThumbRY;
    state.leftTrigger  = xInputState.Gamepad.bLeftTrigger;
    state.rightTrigger = xInputState.Gamepad.bRightTrigger;

}

//==============================================================================
void XInputGamepad::ZeroStateBuffers () {

    SecureZeroMemory(m_stateBuffer, sizeof(m_stateBuffer));

}
