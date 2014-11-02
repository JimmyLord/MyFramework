//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include <wrl.h>
#include <mfapi.h>
#include <mfmediaengine.h>

using namespace Microsoft::WRL;
using namespace Windows::ApplicationModel;

class MediaEngineNotify :
        public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFMediaEngineNotify>
{
public:
    STDMETHODIMP EventNotify(DWORD meEvent, DWORD_PTR param1, DWORD param2)
    {
        return S_OK;
    }
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch DX API errors.
        throw Platform::Exception::CreateException(hr);
    }
}

SoundPlayer::SoundPlayer()
{
    for( int i=0; i<MAX_AUDIO_FILES; i++ )
    {
        m_WaveDescriptors[i].valid = false;
    }
    m_NumAudioBuffersLoaded = 0;

    for( int i=0; i<MAX_CONCURRENT_SOUNDS; i++ )
    {
        m_pSourceVoice[i] = 0;
    }

    m_pAudioEngine = 0;
    HRESULT hr = XAudio2Create( &m_pAudioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR );
    if( hr < 0 )
    {
        LOGError( LOGTag, "XAudio2 - XAudio2Create() failed\n" );
    }
    else
    {
        m_pMasterVoiceEffects = 0;
        hr = m_pAudioEngine->CreateMasteringVoice( &m_pMasterVoiceEffects, 0, 0, 0, 0, 0, AudioCategory_GameEffects );
        if( hr < 0 )
        {
            LOGError( LOGTag, "XAudio2 - CreateMasteringVoice() m_pMasterVoiceEffects failed\n" );
        }

        //m_pMasterVoiceMusic = 0;
        //hr = m_pAudioEngine->CreateMasteringVoice( &m_pMasterVoiceMusic, 2, 48000, 0, 0, 0, AudioCategory_GameMedia );
        //if( hr < 0 )
        //{
        //    LOGError( LOGTag, "XAudio2 - CreateMasteringVoice() m_pMasterVoiceMusic failed\n" );
        //}
    }

    try
    {
        m_mediaEngine = nullptr;

        ComPtr<IMFMediaEngineClassFactory> mediaEngineFactory;
        ComPtr<IMFAttributes> mediaEngineAttributes;

        // Create the class factory for the Media Engine.
        ThrowIfFailed(
            CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mediaEngineFactory))
            );

        // Define configuration attributes.
        ThrowIfFailed(
            MFCreateAttributes(&mediaEngineAttributes, 1)
            );

        ComPtr<MediaEngineNotify> notify = Make<MediaEngineNotify>();
        ComPtr<IUnknown> unknownNotify;
        ThrowIfFailed(
            notify.As(&unknownNotify)
            );

        ThrowIfFailed(
            mediaEngineAttributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, unknownNotify.Get())
            );

        // Create the Media Engine.
        ThrowIfFailed(
            mediaEngineFactory->CreateInstance(0, mediaEngineAttributes.Get(), &m_mediaEngine)
            );

        Platform::String^ musicPath = Platform::String::Concat(
            Package::Current->InstalledLocation->Path, L"\\Data/Audio/WordsInBedRemixMusic.mp3" );

        // Set the music source.
        ThrowIfFailed(
            m_mediaEngine->SetSource(const_cast<wchar_t *>(musicPath->Data()))
            );

        ThrowIfFailed(
            m_mediaEngine->SetLoop(TRUE)
            );
    }
    catch (...)
    {
        m_mediaEngine = nullptr;
    }
}

SoundPlayer::~SoundPlayer()
{
    for( int i=0; i<MAX_CONCURRENT_SOUNDS; i++ )
    {
        if( m_pSourceVoice[i] )
            m_pSourceVoice[i]->DestroyVoice();
    }

    if( m_pMasterVoiceEffects )
        m_pMasterVoiceEffects->DestroyVoice();

    //if( m_pMasterVoiceMusic )
    //    m_pMasterVoiceMusic->DestroyVoice();

    if( m_pAudioEngine )
        m_pAudioEngine->Release();
}

void SoundPlayer::OnFocusGained()
{
}

void SoundPlayer::OnFocusLost()
{
}

void SoundPlayer::PlayMusic(char* path)
{
    if( m_mediaEngine )
    {
        m_mediaEngine->Play(); // Ignore return result, the emulator may error out.
    }
}

void SoundPlayer::PauseMusic()
{
    if( m_mediaEngine )
    {
        m_mediaEngine->Pause(); // Ignore return result, the emulator may error out.
    }
}

void SoundPlayer::UnpauseMusic()
{
    if( m_mediaEngine )
    {
        m_mediaEngine->Play(); // Ignore return result, the emulator may error out.
    }
}

void SoundPlayer::StopMusic()
{
    if( m_mediaEngine )
    {
        m_mediaEngine->Pause(); // Ignore return result, the emulator may error out.
    }
}

int SoundPlayer::LoadSound(const char* buffer, unsigned int buffersize)
{
    //LOGInfo( LOGTag, "WP8 SoundPlayer::LoadSound, buffersize:%d\n", buffersize );

    m_WaveDescriptors[m_NumAudioBuffersLoaded] = WaveLoader::ParseWaveBuffer( buffer, buffersize );
    assert( m_WaveDescriptors[m_NumAudioBuffersLoaded].valid );

    //LOGInfo( LOGTag, "WP8 SoundPlayer parsed, bytes:%d, channels:%d, samples:%d, size:%d\n", 
    //    m_WaveDescriptors[m_NumAudioBuffersLoaded].bytespersample,
    //    m_WaveDescriptors[m_NumAudioBuffersLoaded].numchannels,
    //    m_WaveDescriptors[m_NumAudioBuffersLoaded].samplerate,
    //    m_WaveDescriptors[m_NumAudioBuffersLoaded].datasize );

    m_NumAudioBuffersLoaded++;

    return m_NumAudioBuffersLoaded-1;
}

void SoundPlayer::Shutdown()
{
}

void SoundPlayer::PlaySound(int soundid, bool looping)
{
    // find a free voice:
    int i = 0;
    for( i=0; i<MAX_CONCURRENT_SOUNDS; i++ )
    {
        if( m_pSourceVoice[i] )
        {
            XAUDIO2_VOICE_STATE voicestate;
            m_pSourceVoice[i]->GetState( &voicestate );
            if( voicestate.BuffersQueued == 0 )
            {
                m_pSourceVoice[i]->DestroyVoice();
                m_pSourceVoice[i] = 0;
            }
        }

        if( m_pSourceVoice[i] == 0 )
            break;
    }

    if( i == MAX_CONCURRENT_SOUNDS )
    {
        LOGInfo( LOGTag, "XAUDIO2 - All voices full\n" );
        return;
    }

    if( m_pSourceVoice[i] == 0 )
    {
        WAVEFORMATEXTENSIBLE waveformat = {0}; //WAVEFORMATEX waveformat = {0};
        waveformat.Format.cbSize = 0;
        waveformat.Format.wFormatTag = m_WaveDescriptors[soundid].audioformat;
        waveformat.Format.nChannels = m_WaveDescriptors[soundid].numchannels;
        waveformat.Format.nSamplesPerSec = m_WaveDescriptors[soundid].samplerate;
        waveformat.Format.nAvgBytesPerSec = m_WaveDescriptors[soundid].byterate;
        waveformat.Format.nBlockAlign = m_WaveDescriptors[soundid].blockalign;
        waveformat.Format.wBitsPerSample = m_WaveDescriptors[soundid].bytespersample * 8;

        HRESULT hr = m_pAudioEngine->CreateSourceVoice( &m_pSourceVoice[i], (WAVEFORMATEX*)&waveformat );
        if( hr < 0 )
        {
            LOGError( LOGTag, "XAudio2 - CreateSourceVoice() failed" );
            return;
        }
    }

    //LOGInfo( LOGTag, "XAUDIO2 - Playing with voice %d\n", i );
    XAUDIO2_BUFFER xbuffer = {0};
    xbuffer.AudioBytes = m_WaveDescriptors[soundid].datasize;
    xbuffer.pAudioData = (const BYTE*)m_WaveDescriptors[soundid].data;
    xbuffer.Flags = XAUDIO2_END_OF_STREAM;

    //m_pSourceVoice[i]->Stop();
    //m_pSourceVoice[i]->FlushSourceBuffers();

    HRESULT hr = m_pSourceVoice[i]->SubmitSourceBuffer( &xbuffer );
    if( hr < 0 )
    {
        LOGError( LOGTag, "XAudio2 - SubmitSourceBuffer() failed" );
        return;
    }

    m_pSourceVoice[i]->Start();
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
    m_pAudioEngine->StopEngine();
}

void SoundPlayer::ResumeAll()
{
    m_pAudioEngine->StartEngine();
}
