//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
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

void SoundPlayer::Tick(float deltaTime)
{
    for( int i=0; i<m_NumQueuedSounds; i++ )
    {
        ReallyPlaySound( m_QueuedSounds[i] );
    }

    m_NumQueuedSounds = 0;
}

void SoundPlayer::ReallyPlaySound(int soundid)
{
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "ReallyPlaySound() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return;
    }

    //LOGInfo( LOGTag, "PlaySound - jenv %p - javasoundplayer %p", g_pJavaEnvironment, g_pJavaSoundPlayer );
    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pJavaSoundPlayer );
    //LOGInfo( LOGTag, "PlaySound - cls %p", cls );

    jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "PlaySound", "(II)V" );
    //LOGInfo( LOGTag, "PlaySound - methodid %p soundid %d", methodid, soundid );

    int loop = 0;
    //if( soundid == 21 )
    //    loop = 1;

    g_pJavaEnvironment->CallVoidMethod( g_pJavaSoundPlayer, methodid, soundid, loop );
}

SoundObject* SoundPlayer::LoadSound(const char* path, const char* ext)
{
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "LoadSound() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return 0;
    }

    int i=0;
    for( i=0; i<MAX_SOUNDS; i++ )
    {
        if( m_Sounds[i].m_Sound == 0 )
            break;
    }

    if( i < MAX_SOUNDS )
    {
        //LOGInfo( LOGTag, "LoadSound - jenv %p", g_pJavaEnvironment );
        jclass cls = g_pJavaEnvironment->GetObjectClass( g_pJavaSoundPlayer );
        //LOGInfo( LOGTag, "LoadSound - cls %p", cls );

        char fullpath[MAX_PATH];
        sprintf_s( fullpath, MAX_PATH, "%s%s", path, ext );

        jstring jpath = g_pJavaEnvironment->NewStringUTF( fullpath );

        jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "LoadSound", "(Ljava/lang/String;)I" );
        //LOGInfo( LOGTag, "LoadSound - methodid %p", methodid );
        long soundid = g_pJavaEnvironment->CallIntMethod( g_pJavaSoundPlayer, methodid, jpath );

        // Store the sound id into a soundobject and return the soundobject.
        m_Sounds[i].m_Sound = soundid;
        return &m_Sounds[i];
    }

    return 0;
}

SoundObject* SoundPlayer::LoadSound(const char* fullpath)
{
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "LoadSound() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return 0;
    }

    int i=0;
    for( i=0; i<MAX_SOUNDS; i++ )
    {
        if( m_Sounds[i].m_Sound == 0 )
            break;
    }

    if( i < MAX_SOUNDS )
    {
        //LOGInfo( LOGTag, "LoadSound - jenv %p", g_pJavaEnvironment );
        jclass cls = g_pJavaEnvironment->GetObjectClass( g_pJavaSoundPlayer );
        //LOGInfo( LOGTag, "LoadSound - cls %p", cls );

        jstring jpath = g_pJavaEnvironment->NewStringUTF( fullpath );

        jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "LoadSound", "(Ljava/lang/String;)I" );
        //LOGInfo( LOGTag, "LoadSound - methodid %p", methodid );
        long soundid = g_pJavaEnvironment->CallIntMethod( g_pJavaSoundPlayer, methodid, jpath );

        // store the sound id into a soundobject and return the soundobject.
        m_Sounds[i].m_Sound = soundid;
        return &m_Sounds[i];
    }

    return 0;
}

void SoundPlayer::Shutdown()
{
}

int SoundPlayer::PlaySound(SoundObject* pSoundObject)
{
    //LOGInfo( LOGTag, "PlaySound - %d", soundid );

    if( m_NumQueuedSounds < MAX_QUEUED_SOUNDS )
    {
        m_QueuedSounds[m_NumQueuedSounds] = pSoundObject->m_Sound;
        m_NumQueuedSounds++;
    }

    return 0; // TODO: Should return some unique id for the sound, so it can be stopped/paused/etc.
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
