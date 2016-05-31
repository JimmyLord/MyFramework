//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GamepadManager_H__
#define __GamepadManager_H__

enum MyGamepadButtons
{
    MyGamePad_DPadUp          = 0x00000001,
    MyGamePad_DPadDown        = 0x00000002,
    MyGamePad_DPadLeft        = 0x00000004,
    MyGamePad_DPadRight       = 0x00000008,
    MyGamePad_A               = 0x00000010,
    MyGamePad_B               = 0x00000020,
    MyGamePad_X               = 0x00000040,
    MyGamePad_Y               = 0x00000080,
    MyGamePad_LeftBumper      = 0x00000100,
    MyGamePad_RightBumper     = 0x00000200,
    MyGamePad_Start           = 0x00000400,
    MyGamePad_Back            = 0x00000800,
    MyGamePad_LeftStick       = 0x00001000,
    MyGamePad_RightStick      = 0x00002000,
};

struct GamepadState
{
    bool connected;

    Vector2 leftstick;
    Vector2 rightstick;
    int buttons; // stores up to 32 MyGamepadButtons
    float lefttrigger;
    float righttrigger;

    void Reset()
    {
        connected = false;

        leftstick.Set( 0, 0 );
        rightstick.Set( 0, 0 );
        buttons = 0;
        lefttrigger = 0;
        righttrigger = 0;
    }
};

class GamepadManager;

extern GamepadManager* g_pGamepadManager;

class GamepadManager
{
public:
    static const int MAX_GAMEPADS = 4;

protected:
    GamepadState m_OldGamepadStates[MAX_GAMEPADS];
    GamepadState m_CurrentGamepadStates[MAX_GAMEPADS];

public:
    GamepadManager();
    virtual ~GamepadManager();

    virtual void Initialize() = 0;
    virtual void Tick(double TimePassed) = 0;
};

#endif //__GamepadManager_H__
