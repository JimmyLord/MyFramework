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

//====================================================================================================
// SoundObject
//====================================================================================================
SoundObject::SoundObject()
{
    m_pFile = 0;
    m_WaveDesc.valid = false;
}

cJSON* SoundObject::ExportAsJSONObject()
{
    MyAssert( m_pFile );

    cJSON* jSound = cJSON_CreateObject();

    cJSON_AddStringToObject( jSound, "Path", GetFullPath() );

    return jSound;
}

const char* SoundObject::GetFullPath()
{
    return m_pFile->GetFullPath();
}

void SoundObject::CreateSourceVoice(IXAudio2* pEngine)
{
    WAVEFORMATEX waveformat;
    waveformat.wFormatTag = m_WaveDesc.audioformat;
    waveformat.nChannels = (WORD)m_WaveDesc.numchannels;
    waveformat.nSamplesPerSec = m_WaveDesc.samplerate;
    waveformat.nAvgBytesPerSec = m_WaveDesc.byterate;
    waveformat.nBlockAlign = m_WaveDesc.blockalign;
    waveformat.wBitsPerSample = (WORD)m_WaveDesc.bytespersample * 8;
    waveformat.cbSize = 0;

    m_XAudioBuffer.Flags = XAUDIO2_END_OF_STREAM;
    m_XAudioBuffer.AudioBytes = m_WaveDesc.datasize;
    m_XAudioBuffer.pAudioData = (const BYTE*)m_WaveDesc.data;
    m_XAudioBuffer.PlayBegin = 0;
    m_XAudioBuffer.PlayLength = 0;
    m_XAudioBuffer.LoopBegin = 0;
    m_XAudioBuffer.LoopLength = 0;
    m_XAudioBuffer.LoopCount = 0;
    m_XAudioBuffer.pContext = 0;
}

//====================================================================================================
// SoundChannel
//====================================================================================================
SoundChannel::SoundChannel()
{
    m_pSourceVoice = 0;

    m_CurrentState = SoundChannelState_Free;
    m_TimePlaybackStarted = 0;
}

void SoundChannel::PlaySound(SoundObject* pSoundObject)
{
    m_pSourceVoice->Stop();
	m_pSourceVoice->FlushSourceBuffers();
    m_pSourceVoice->Start();
    m_pSourceVoice->SubmitSourceBuffer( &pSoundObject->m_XAudioBuffer );

    m_TimePlaybackStarted = MyTime_GetUnpausedTime();
}

void SoundChannel::StopSound()
{
    m_pSourceVoice->Stop();
	m_pSourceVoice->FlushSourceBuffers();

    m_TimePlaybackStarted = 0;
}

//====================================================================================================
// SoundPlayer
//====================================================================================================
SoundPlayer::SoundPlayer()
{
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    HRESULT result = XAudio2Create( &m_pEngine );
    
    if( result == S_OK )
        result = m_pEngine->CreateMasteringVoice( &m_pMasteringVoice );

    if( result != S_OK )
    {
        LOGError( LOGTag, "Error initializing XAudio\n" );
        exit( 1 );
    }

    for( int i=0; i<MAX_CHANNELS; i++ )
    {
        IXAudio2SourceVoice* pSourceVoice;

        // Hardcoded for now, all wav's must be this format
        WAVEFORMATEX waveformat;
        waveformat.wFormatTag = 1;
        waveformat.nChannels = 1;
        waveformat.nSamplesPerSec = 44100;
        waveformat.nAvgBytesPerSec = 88200;
        waveformat.nBlockAlign = 2;
        waveformat.wBitsPerSample = 16;
        waveformat.cbSize = 0;

        int result = m_pEngine->CreateSourceVoice( &pSourceVoice, &waveformat );
        if( result != S_OK )
        {
            LOGError( LOGTag, "Unable to create source voice\n" );
        }
        else
        {
            m_Channels[i].SetSourceVoice( pSourceVoice );
        }
    }
}

SoundPlayer::~SoundPlayer()
{
    for( int i=0; i<MAX_SOUNDS; i++ )
    {
        if( m_Sounds[i].m_pFile )
            m_Sounds[i].m_pFile->Release();
    }

    m_pEngine->Release();
    CoUninitialize();
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

//SoundObject* SoundPlayer::LoadSound(const char* path, const char* ext)
//{
//    return 0;
//}

SoundObject* SoundPlayer::LoadSound(const char* fullpath)
{
    MyFileObject* pFile = g_pFileManager->RequestFile( fullpath );
    return LoadSound( pFile );
}

SoundObject* SoundPlayer::LoadSound(MyFileObject* pFile)
{
    int i=0;
    for( i=0; i<MAX_SOUNDS; i++ )
    {
        if( m_Sounds[i].m_pFile == 0 )
            break;
    }

    if( i < MAX_SOUNDS )
    {
        // store the wave file and wave desc into a soundobject and return the soundobject.
        // file may not be fully loaded, so m_WaveDesc.valid == false
        //    wave file will attempt to be parsed again in SoundPlayer::PlaySound once file is loaded

        m_Sounds[i].m_pFile = pFile;
        m_Sounds[i].m_WaveDesc.valid = false;

        if( pFile->IsFinishedLoading() )
        {
            m_Sounds[i].m_WaveDesc = WaveLoader::ParseWaveBuffer( pFile->GetBuffer(), pFile->GetFileLength() );
            if( m_Sounds[i].m_WaveDesc.valid == false )
            {
                LOGError( LOGTag, "WAV file parsing failed (%s)\n", pFile->GetFullPath() );
            }
            else
            {
                m_Sounds[i].CreateSourceVoice( m_pEngine );
            }
        }

        return &m_Sounds[i];
    }

    return 0;
}

void SoundPlayer::Shutdown()
{
}

int SoundPlayer::PlaySound(SoundObject* pSoundObject)
{
    // Attempt to parse the file again in case it wasn't loaded the first time.
    if( pSoundObject->m_WaveDesc.valid == false )
    {
        MyAssert( pSoundObject->m_pFile );
        if( pSoundObject->m_pFile->IsFinishedLoading() )
        {
            pSoundObject->m_WaveDesc = WaveLoader::ParseWaveBuffer( pSoundObject->m_pFile->GetBuffer(), pSoundObject->m_pFile->GetFileLength() );
            if( pSoundObject->m_WaveDesc.valid == false )
            {
                LOGError( LOGTag, "WAV file parsing failed (%s)\n", pSoundObject->m_pFile->GetFullPath() );
            }
            else
            {
                pSoundObject->CreateSourceVoice( m_pEngine );
            }
        }
    }

    if( pSoundObject->m_WaveDesc.valid == false )
    {
        return -1; // sound didn't play
    }

    // find a free channel
    int channelindex;
    for( channelindex = 0; channelindex < MAX_CHANNELS; channelindex++ )
    {
        if( m_Channels[channelindex].GetState() == SoundChannel::SoundChannelState_Free )
            break;
    }

    // if all channels are in use, find and use the oldest sound channel
    if( channelindex == -1 )
    {
        double oldesttime = DBL_MAX;
        for( int i = 0; i < MAX_CHANNELS; i++ )
        {
            double thistime = m_Channels[channelindex].GetTimePlaybackStarted();
            if( thistime < oldesttime )
            {
                oldesttime = thistime;
                channelindex = i;
            }
        }
    }

    m_Channels[channelindex].PlaySound( pSoundObject );

    return channelindex;

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
    m_Channels[channel].StopSound();
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
