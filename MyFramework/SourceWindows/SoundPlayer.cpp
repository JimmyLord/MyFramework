//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "../SourceWindows/SDL-1.2.15/include/SDL.h"
#include "../SourceWindows/SDL_mixer-1.2.12/include/SDL_mixer.h"

cJSON* SoundObject::ExportAsJSONObject()
{
    cJSON* jSound = cJSON_CreateObject();

    cJSON_AddStringToObject( jSound, "Path", m_FullPath );

    return jSound;
}

SoundPlayer::SoundPlayer()
{
    int audio_rate = 22050;
    Uint16 audio_format = AUDIO_S16;
    int audio_channels = 2;
    int audio_buffers = 512; //4096;

    SDL_Init( SDL_INIT_AUDIO );

    if( Mix_OpenAudio( audio_rate, audio_format, audio_channels, audio_buffers ) )
    {
        printf( "Unable to open audio!\n" );
        exit( 1 );
    }

    int numchannels = Mix_GroupCount( -1 );

    Mix_GroupChannel( 0, SoundGroup_Music );
    Mix_GroupChannels( 1, numchannels-1, SoundGroup_Effects );

    //for( int i=0; i<MAX_SOUNDS; i++ )
    //{
    //    m_Sounds[i] = 0;
    //}

    //m_Music = 0;
}

SoundPlayer::~SoundPlayer()
{
    Mix_CloseAudio();
    SDL_Quit();
}

void SoundPlayer::OnFocusGained()
{
}

void SoundPlayer::OnFocusLost()
{
}

void SoundPlayer::PlayMusic(char* path)
{
    m_Music.m_Sound = Mix_LoadWAV( path );//"sound/Music.wav" );
    //strcpy_s( m_Music.m_FullPath, MAX_PATH, path );
    Mix_PlayChannel( 0, m_Music.m_Sound, -1 );
}

void SoundPlayer::PauseMusic()
{
}

void SoundPlayer::UnpauseMusic()
{
}

void SoundPlayer::StopMusic()
{
    Mix_HaltChannel( 0 );
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
        char fullpath[MAX_PATH];
        sprintf_s( fullpath, MAX_PATH, "%s%s", path, ext );
        m_Sounds[i].m_Sound = Mix_LoadWAV( fullpath );
        //strcpy_s( m_Sounds[i].m_FullPath, MAX_PATH, fullpath );
        return &m_Sounds[i];
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
        m_Sounds[i].m_Sound = Mix_LoadWAV( fullpath );
        strcpy_s( m_Sounds[i].m_FullPath, MAX_PATH, fullpath );
        return &m_Sounds[i];
    }

    return 0;
}

void SoundPlayer::Shutdown()
{
}

int SoundPlayer::PlaySound(SoundObject* pSoundObject)
{
    int channel = Mix_GroupAvailable( SoundGroup_Effects );
    if( channel == -1 )
        channel = Mix_GroupOldest( SoundGroup_Effects );

    Mix_PlayChannel( channel, pSoundObject->m_Sound, 0 );

    return channel;
}

int SoundPlayer::PlaySound(int soundid)
{
    if( soundid == -1 )
        return -1;

    int channel = Mix_GroupAvailable( SoundGroup_Effects );
    if( channel == -1 )
        channel = Mix_GroupOldest( SoundGroup_Effects );

    Mix_PlayChannel( channel, m_Sounds[soundid].m_Sound, 0 );

    return channel;
}

void SoundPlayer::StopSound(int channel)
{
    Mix_HaltChannel( channel );
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
