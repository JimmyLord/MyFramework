//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "../SourceCommon/CommonHeader.h"

SoundPlayer::SoundPlayer()
{
    m_NumQueuedSounds = 0;
}

SoundPlayer::~SoundPlayer()
{
}

void SoundPlayer::OnFocusGained()
{
}

void SoundPlayer::OnFocusLost()
{
}

void SoundPlayer::Tick(double TimePassed)
{
}

void SoundPlayer::ReallyPlaySound(int soundid)
{
}

SoundObject* SoundPlayer::LoadSound(const char* path, const char* ext)
{
    int i=0;
    for( i=0; i<MAX_SOUNDS; i++ )
    {
        if( m_Sounds[i].m_Sound == 0 )
            break;
    }

    if( i < MAX_SOUNDS )
    {
        // store the sound id into a soundobject and return the soundobject.
        //m_Sounds[i].m_Sound = soundid;
        //return &m_Sounds[i];
    }

    return 0;
}

SoundObject* SoundPlayer::LoadSound(const char* fullpath)
{
    int i=0;
    for( i=0; i<MAX_SOUNDS; i++ )
    {
        if( m_Sounds[i].m_Sound == 0 )
            break;
    }

    if( i < MAX_SOUNDS )
    {
        // store the sound id into a soundobject and return the soundobject.
        //m_Sounds[i].m_Sound = soundid;
        //return &m_Sounds[i];
    }

    return 0;
}

void SoundPlayer::Shutdown()
{
}

int SoundPlayer::PlaySound(SoundObject* pSoundObject)
{
    //LOGInfo( LOGTag, "PlaySound - %d", soundid );

    return 0; // TODO: should return some unique id for the sound, so it can be stopped/paused/etc.
}

void SoundPlayer::StopSound(int soundid)
{
}

void SoundPlayer::PauseSound(int soundid)
{
}

void SoundPlayer::ResumeSound(int soundid)
{
}

void SoundPlayer::PauseAll()
{
}

void SoundPlayer::ResumeAll()
{
}
