//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "../SourceCommon/Input/GamepadManager.h"
#include "GamepadManagerXInput.h"
#include <Xinput.h>

GamepadManagerXInput::GamepadManagerXInput()
{
}

void GamepadManagerXInput::Initialize()
{
    GamepadManager::Initialize();
}

void GamepadManagerXInput::Tick(double TimePassed)
{
    for( DWORD i=0; i<MAX_GAMEPADS; i++ )
    {
        XINPUT_STATE state;
        ZeroMemory( &state, sizeof(XINPUT_STATE) );

        // get the state of the controller from XInput
        DWORD dwResult = XInputGetState( i, &state );

        if( dwResult == ERROR_SUCCESS )
        {
            m_OldGamepadStates[i] = m_CurrentGamepadStates[i];

            Vector2 leftstick = Vector2(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY);
            NormalizeStick( leftstick, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, &m_CurrentGamepadStates[i].leftstick );

            Vector2 rightstick = Vector2(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
            NormalizeStick( rightstick, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, &m_CurrentGamepadStates[i].rightstick );

            //float lefttrigger = state.Gamepad.bLeftTrigger;
            //NormalizeTrigger( lefttrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, &m_CurrentGamepadStates[i].lefttrigger );

            //float righttrigger = state.Gamepad.bRightTrigger;
            //NormalizeTrigger( righttrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, &m_CurrentGamepadStates[i].righttrigger );
        }
    }
}

void GamepadManagerXInput::NormalizeStick(Vector2 stick, float deadzone, Vector2* stickout)
{
    //           deadzone
    //-32k     -8k      8k       32k <- values in
    //  (--------(      )--------)
    // -1        0      0        1   <- values out

    // calculate how far the stick is pushed, in 0-32k range
    float magnitude = stick.Length();

    // if we're inside the deadzone, zero out the input
    if( magnitude < deadzone )
    {
        stickout->Set( 0, 0 );
    }
    else // normalize the input to match image above
    {
        // clamp the magnitude to its expected range
        if( magnitude > 32767 )
            magnitude = 32767;

        float normalizedmagnitude = (magnitude - deadzone) / (32767 - deadzone);
        Vector2 normalizedstick = stick / magnitude;

        *stickout = normalizedstick * normalizedmagnitude;
    }
}
