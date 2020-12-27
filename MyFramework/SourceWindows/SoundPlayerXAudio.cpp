//
// Copyright (c) 2017-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "SoundPlayerXAudio.h"
#include "../SourceCommon/DataTypes/MyActivePool.h"
#include "../SourceCommon/Helpers/MyFileObject.h"
#include "../SourceCommon/Helpers/FileManager.h"
#include "../SourceCommon/Helpers/MyTime.h"

//====================================================================================================
// SoundObject
//====================================================================================================
SoundObject::SoundObject()
{
    m_pFile = 0;
    m_WaveDesc.valid = false;

    m_pSourcePool = 0;

#if _DEBUG
    m_BaseCount = 1; // RefCount hack: since soundobjects are in an array in soundplayer, final ref won't be released.
#endif
}

SoundObject::~SoundObject()
{
}

void SoundObject::Release() // override from RefCount
{
    RefCount::Release();

    if( m_RefCount == 1 )
    {
        m_pSourcePool->ReturnObjectToPool( this );

        //this->Remove(); // remove from cpplist

        SAFE_RELEASE( m_pFile );
        m_WaveDesc.valid = false;
        m_XAudioBuffer.pAudioData = 0;
    }
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

void SoundObject::SetFile(MyFileObject* pFile)
{
    m_pFile = pFile;
    pFile->AddRef();
    m_WaveDesc.valid = false;
}

void SoundObject::Init()
{
    MyAssert( m_pFile );

    if( m_pFile->IsFinishedLoading() )
    {
        m_WaveDesc = WaveLoader::ParseWaveBuffer( m_pFile->GetBuffer(), m_pFile->GetFileLength() );

        if( m_WaveDesc.valid == false )
        {
            LOGError( LOGTag, "WAV file parsing failed (%s)\n", m_pFile->GetFullPath() );
        }
        else
        {
            //WAVEFORMATEX waveformat;
            //waveformat.wFormatTag = m_WaveDesc.audioformat;
            //waveformat.nChannels = (WORD)m_WaveDesc.numchannels;
            //waveformat.nSamplesPerSec = m_WaveDesc.samplerate;
            //waveformat.nAvgBytesPerSec = m_WaveDesc.byterate;
            //waveformat.nBlockAlign = m_WaveDesc.blockalign;
            //waveformat.wBitsPerSample = (WORD)m_WaveDesc.bytespersample * 8;
            //waveformat.cbSize = 0;

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
    }
}

//====================================================================================================
// SoundChannel
//====================================================================================================
SoundChannel::SoundChannel()
{
    m_pSourceVoice = 0;
    m_pVoiceCallback = 0;

    m_CurrentState = SoundChannelState_Free;
    m_TimePlaybackStarted = 0;
}

SoundChannel::~SoundChannel()
{
    delete m_pVoiceCallback;
}

void SoundChannel::PlaySound(SoundObject* pSoundObject)
{
    m_pSourceVoice->Stop();
	m_pSourceVoice->FlushSourceBuffers();
    m_pSourceVoice->Start();
    m_pSourceVoice->SubmitSourceBuffer( pSoundObject->GetXAudioBuffer() );

    m_TimePlaybackStarted = MyTime_GetUnpausedTime();
}

void SoundChannel::StopSound()
{
    m_CurrentState = SoundChannelState_Free;

    m_pSourceVoice->Stop();
	m_pSourceVoice->FlushSourceBuffers();

    m_TimePlaybackStarted = 0;
}

//====================================================================================================
// SoundPlayer
//====================================================================================================
SoundPlayer::SoundPlayer()
{
    // Initialize all pointers to null in case of early exit (mainly for debugging).
    m_pEngine = nullptr;
    m_pMasteringVoice = nullptr;
    for( int i=0; i<MAX_CHANNELS; i++ )
        m_pChannels[i] = nullptr;

    // Allocate our objects.
    m_SoundObjectPool.AllocateObjects( NUM_SOUNDOBJECTS_IN_POOL );

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

        m_pChannels[i] = new SoundChannel();

        VoiceCallback* pVoiceCallback = new VoiceCallback( m_pChannels[i] );
        int result = m_pEngine->CreateSourceVoice( &pSourceVoice, &waveformat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, pVoiceCallback );
        if( result != S_OK )
        {
            delete pVoiceCallback;
            LOGError( LOGTag, "Unable to create source voice\n" );
        }
        else
        {
            m_pChannels[i]->SetSourceVoice( pSourceVoice, pVoiceCallback );
        }
    }
}

SoundPlayer::~SoundPlayer()
{
    for( int i=0; i<MAX_CHANNELS; i++ )
    {
        delete m_pChannels[i];
    }

    //SoundObject* pSound = 0;
    //while( pSound = m_pSounds.GetHead() )
    //{
    //    pSound->Release();
    //}

    if( m_pEngine != nullptr )
    {
        m_pEngine->Release();
    }

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

SoundObject* SoundPlayer::LoadSound(FileManager* pFileManager, const char* fullpath)
{
    MyFileObject* pFile = pFileManager->RequestFile( fullpath );
    SoundObject* pSoundObject = LoadSound( pFile );
    pFile->Release();

    return pSoundObject;
}

SoundObject* SoundPlayer::LoadSound(MyFileObject* pFile)
{
    SoundObject* pSound = m_SoundObjectPool.GetObjectFromPool();

    if( pSound == 0 )
    {
        LOGError( LOGTag, "Sound pool empty, too many sounds loaded at one time?\n" );
        return 0;
    }
    
    // TODO: avoid duplicate sounds being loaded.

    //m_pSounds.AddTail( pSound );

    pSound->SetSourcePool( &m_SoundObjectPool );
    pSound->AddRef();

    // store the wave file and wave desc into a soundobject and return the soundobject.
    // file may not be fully loaded, so m_WaveDesc.valid == false
    //    wave file will attempt to be parsed again in SoundPlayer::PlaySound once file is loaded

    pSound->SetFile( pFile );
    pSound->Init();

    return pSound;
}

void SoundPlayer::Shutdown()
{
}

int SoundPlayer::FindFreeChannel()
{
    int channelIndex = -1;

    for( channelIndex = 0; channelIndex < MAX_CHANNELS; channelIndex++ )
    {
        if( m_pChannels[channelIndex]->GetState() == SoundChannel::SoundChannelState_Free )
            break;
    }

    // If all channels are in use, find and use the oldest sound channel
    if( channelIndex == MAX_CHANNELS )
    {
        channelIndex = FindOldestChannel();
    }

    return channelIndex;
}

int SoundPlayer::FindOldestChannel()
{
    int channelIndex;

    double oldestTime = DBL_MAX;
    for( int i = 0; i < MAX_CHANNELS; i++ )
    {
        double thisTime = m_pChannels[i]->GetTimePlaybackStarted();
        if( thisTime < oldestTime )
        {
            oldestTime = thisTime;
            channelIndex = i;
        }
    }

    return channelIndex;
}

int SoundPlayer::PlaySound(SoundObject* pSoundObject)
{
    // Attempt to parse the file again in case it wasn't loaded the first time.
    if( pSoundObject->IsValid() == false )
    {
        pSoundObject->Init();
    }

    if( pSoundObject->IsValid() == false )
    {
        return -1; // sound didn't play
    }

    // Find a free or oldest channel.
    int channelIndex = FindFreeChannel();

    // If no channel was found (should be impossible since we'll also accept oldest channel)
    if( channelIndex == -1 )
        return -1;

    m_pChannels[channelIndex]->PlaySound( pSoundObject );

    return channelIndex;
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
    m_pChannels[channel]->StopSound();
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
