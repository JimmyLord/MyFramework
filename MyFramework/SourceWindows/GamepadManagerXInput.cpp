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
        }
    }
}
