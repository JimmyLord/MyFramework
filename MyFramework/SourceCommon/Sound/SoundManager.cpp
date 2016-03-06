//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

void SoundCue::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_SoundCuePointer;
    g_DragAndDropStruct.m_Value = this;
}

SoundManager::SoundManager()
{
    m_SoundCuePool.AllocateObjects( NUM_SOUND_CUES_TO_POOL );
}

SoundManager::~SoundManager()
{
}

SoundCue* SoundManager::CreateCue(const char* name)
{
    SoundCue* pCue = m_SoundCuePool.GetObject();

    strcpy_s( pCue->m_Name, MAX_SOUND_CUE_NAME_LEN, name );
    m_Cues.AddTail( pCue );

    g_pPanelMemory->AddSoundCue( pCue, "Default", name, SoundCue::StaticOnDrag );

    return pCue;
}

void SoundManager::AddSoundToCue(SoundCue* pCue, const char* fullpath)
{
    SoundObject* pSoundObject = g_pGameCore->m_pSoundPlayer->LoadSound( fullpath );
    pCue->m_SoundObjects.AddTail( pSoundObject );

    g_pPanelMemory->AddSoundObject( pSoundObject, pCue, fullpath, 0 );
}

SoundCue* SoundManager::FindCueByName(const char* name)
{
    for( CPPListNode* pNode = m_Cues.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        SoundCue* pCue = (SoundCue*)pNode;

        if( strcmp( pCue->m_Name, name ) == 0 )
        {
            return pCue;
        }
    }

    return 0;
}

int SoundManager::PlayCueByName(const char* name)
{
    SoundCue* pCue = FindCueByName( name );
    if( pCue )
    {
        return PlayCue( pCue );
    }

    return -1;
}

int SoundManager::PlayCue(SoundCue* pCue)
{
    MyAssert( pCue );

    SoundObject* pSoundObject = (SoundObject*)pCue->m_SoundObjects.GetHead();
    return g_pGameCore->m_pSoundPlayer->PlaySound( pSoundObject );
}
