//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#pragma warning(disable:4005) // xaudio includes urlmon.h which was already included by something earlier.
#include <xaudio2.h>

SoundObject::SoundObject()
{
    m_pFile = 0;
    m_WaveDesc.valid = false;

    m_pSourceVoice = 0;
}

cJSON* SoundObject::ExportAsJSONObject()
{
    MyAssert( m_pFile );

    cJSON* jSound = cJSON_CreateObject();

    cJSON_AddStringToObject( jSound, "Path", GetFullPath() );

    return jSound;
}

SoundPlayer::SoundPlayer()
{
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    HRESULT result = XAudio2Create( &m_pEngine );
    
    if( result == S_OK )
        result = m_pEngine->CreateMasteringVoice( &m_pMasteringVoice );

    if( result != S_OK )
    {
        printf( "Unable to open audio!\n" );
        exit( 1 );
    }

    //m_pEngine->CreateSourceVoice( &m_pSourceVoice, buffer.wf() )
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

void SoundPlayer::PlayMusic(char* path)
{
}

void SoundPlayer::PauseMusic()
{
}

void SoundPlayer::UnpauseMusic()
{
}

void SoundPlayer::StopMusic()
{
}

SoundObject* SoundPlayer::LoadSound(const char* path, const char* ext)
{
    //int i=0;
    //for( i=0; i<MAX_SOUNDS; i++ )
    //{
    //    if( m_Sounds[i].m_Sound == 0 )
    //        break;
    //}

    //if( i < MAX_SOUNDS )
    //{
    //    char fullpath[MAX_PATH];
    //    sprintf_s( fullpath, MAX_PATH, "%s%s", path, ext );
    //    m_Sounds[i].m_Sound = Mix_LoadWAV( fullpath );
    //    //strcpy_s( m_Sounds[i].m_FullPath, MAX_PATH, fullpath );
    //    return &m_Sounds[i];
    //}

    return 0;
}

SoundObject* SoundPlayer::LoadSound(const char* fullpath)
{
    //int i=0;
    //for( i=0; i<MAX_SOUNDS; i++ )
    //{
    //    if( m_Sounds[i].m_Sound == 0 )
    //        break;
    //}

    //if( i < MAX_SOUNDS )
    //{
    //    m_Sounds[i].m_Sound = Mix_LoadWAV( fullpath );
    //    strcpy_s( m_Sounds[i].m_FullPath, MAX_PATH, fullpath );
    //    return &m_Sounds[i];
    //}

    return 0;
}

void SoundPlayer::Shutdown()
{
}

int SoundPlayer::PlaySound(SoundObject* pSoundObject)
{
    //int channel = Mix_GroupAvailable( SoundGroup_Effects );
    //if( channel == -1 )
    //    channel = Mix_GroupOldest( SoundGroup_Effects );

    //Mix_PlayChannel( channel, pSoundObject->m_Sound, 0 );

    return 0; //channel;
}

int SoundPlayer::PlaySound(int soundid)
{
    //if( soundid == -1 )
    //    return -1;

    //int channel = Mix_GroupAvailable( SoundGroup_Effects );
    //if( channel == -1 )
    //    channel = Mix_GroupOldest( SoundGroup_Effects );

    //Mix_PlayChannel( channel, m_Sounds[soundid].m_Sound, 0 );

    return 0; //channel;
}

void SoundPlayer::StopSound(int channel)
{
    //Mix_HaltChannel( channel );
}

void SoundPlayer::PauseSound(int channel)
{
}

void SoundPlayer::ResumeSound(int channel)
{
}

void SoundPlayer::PauseAll()
{
}

void SoundPlayer::ResumeAll()
{
}
