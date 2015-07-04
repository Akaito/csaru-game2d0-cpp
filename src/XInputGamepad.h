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

class XInputGamepad {

public: // Constants

    enum EBufferSlot {
        BUFFER_SLOT_CURRENT = 0,
        BUFFER_SLOT_PREVIOUS,
        BUFFER_SLOTS
    };
    
    enum EButtonFlags {
        BUTTON_FLAG_DPAD_UP        = 1 <<  0,
        BUTTON_FLAG_DPAD_DOWN      = 1 <<  1,
        BUTTON_FLAG_DPAD_LEFT      = 1 <<  2,
        BUTTON_FLAG_DPAD_RIGHT     = 1 <<  3,
        BUTTON_FLAG_START          = 1 <<  4,
        BUTTON_FLAG_BACK           = 1 <<  5,
        BUTTON_FLAG_LEFT_STICK     = 1 <<  6,
        BUTTON_FLAG_RIGHT_STICK    = 1 <<  7,
        BUTTON_FLAG_LEFT_SHOULDER  = 1 <<  8,
        BUTTON_FLAG_RIGHT_SHOULDER = 1 <<  9,
        BUTTON_FLAG_A              = 1 << 12,
        BUTTON_FLAG_B              = 1 << 13,
        BUTTON_FLAG_X              = 1 << 14,
        BUTTON_FLAG_Y              = 1 << 15,
    };
    
    enum EAnalogTypes {
        ANALOG_TYPE_LEFT_STICK_X = 0,
        ANALOG_TYPE_LEFT_STICK_Y,
        ANALOG_TYPE_RIGHT_STICK_X,
        ANALOG_TYPE_RIGHT_STICK_Y,
        ANALOG_TYPE_LEFT_TRIGGER,
        ANALOG_TYPE_RIGHT_TRIGGER,
        ANALOG_TYPES
    };
    
    struct ControllerState {
        unsigned short buttons;
        short          leftStickX;
        short          leftStickY;
        short          rightStickX;
        short          rightStickY;
        unsigned char  leftTrigger;
        unsigned char  rightTrigger;
    };

private: // Data

    unsigned        m_controllerIndex;
    bool            m_isConnected;
    unsigned        m_lastPacketNumber;
    ControllerState m_stateBuffer[BUFFER_SLOTS];

private: // Helpers

    void AdvanceStateBuffers ();
    void ZeroStateBuffers ();
    
    void OnDisconnect ();
    void OnConnect ();

public: // Construction

    XInputGamepad ();
    virtual ~XInputGamepad ();
    
public: // Queries

    bool  IsConnected () const { return m_isConnected; };
    bool  AreButtonsPressed (EButtonFlags buttonFlags) const;
    float GetAnalogValueAsFloat (EAnalogTypes analogType) const;
    float GetLeftStickXAsFloat () const;
    float GetLeftStickYAsFloat () const;
    float GetLeftTriggerAsFloat () const;
    float GetRightStickXAsFloat () const;
    float GetRightStickYAsFloat () const;
    float GetRightTriggerAsFloat () const;

public: // Commands

    void Update ();

};
