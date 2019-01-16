//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "../SourceCommon/Core/GameCore.h"
#include "../SourceCommon/Sound/SoundManager.h"
#include "../SourceEditor/CommandStack.h"
#include "../SourceEditor/EditorCommands.h"

//====================================================================================================
// EditorCommand_UnloadSoundCues
//====================================================================================================

EditorCommand_UnloadSoundCues::EditorCommand_UnloadSoundCues(const std::vector<SoundCue*>& selectedsoundcues)
{
    m_Name = "EditorCommand_UnloadSoundCues";

    MyAssert( selectedsoundcues.size() > 0 );

    for( unsigned int i=0; i<selectedsoundcues.size(); i++ )
    {
        SoundCue* pSoundCue = selectedsoundcues[i];
        m_SoundCues.push_back( pSoundCue );

        pSoundCue->AddRef();
    }

    m_ReleaseSoundCuesWhenDestroyed = false;
}

EditorCommand_UnloadSoundCues::~EditorCommand_UnloadSoundCues()
{
    if( m_ReleaseSoundCuesWhenDestroyed )
    {
        for( unsigned int i=0; i<m_SoundCues.size(); i++ )
        {
            m_SoundCues[i]->Release();
        }
    }
}

void EditorCommand_UnloadSoundCues::Do()
{
    for( unsigned int i=0; i<m_SoundCues.size(); i++ )
    {
        SoundCue* pSoundCue = m_SoundCues[i];

        g_pGameCore->GetSoundManager()->UnloadCue( m_SoundCues[i] );
    }

    m_ReleaseSoundCuesWhenDestroyed = true;
}

void EditorCommand_UnloadSoundCues::Undo()
{
    for( unsigned int i=0; i<m_SoundCues.size(); i++ )
    {
        SoundCue* pSoundCue = m_SoundCues[i];

        g_pGameCore->GetSoundManager()->LoadExistingCue( m_SoundCues[i] );
    }

    m_ReleaseSoundCuesWhenDestroyed = false;
}

EditorCommand* EditorCommand_UnloadSoundCues::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
//====================================================================================================
