//
// Copyright (c) 2017-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SoundPlayerXAudio_H__
#define __SoundPlayerXAudio_H__

#include "../SourceCommon/DataTypes/MyActivePool.h"
#include "../SourceCommon/Sound/WaveLoader.h"

class MyFileObject;
class SoundChannel;
class VoiceCallback;
class SoundPlayer;

struct IXAudio2;
struct IXAudio2SourceVoice;
struct IXAudio2MasteringVoice;

class SoundObject : public TCPPListNode<SoundObject*>, public RefCount
{
protected:
    MyFileObject* m_pFile;
    MyWaveDescriptor m_WaveDesc; // contains pointer to data in fileobject buffer
    XAUDIO2_BUFFER m_XAudioBuffer;

    MySimplePool<SoundObject>* m_pSourcePool;

public:
    SoundObject();
    ~SoundObject();

    virtual void Release(); // override from RefCount

    cJSON* ExportAsJSONObject();
    const char* GetFullPath();

    void SetFile(MyFileObject* pFile);
    void Init();

    void SetSourcePool(MySimplePool<SoundObject>* pSourcePool) { m_pSourcePool = pSourcePool; }
    XAUDIO2_BUFFER* GetXAudioBuffer() { return &m_XAudioBuffer; }
    bool IsValid() { return m_WaveDesc.valid; }
};

class SoundChannel
{
public:
    enum SoundChannelStates
    {
        SoundChannelState_Free,
        SoundChannelState_Playing,
        SoundChannelState_Paused,
    };

protected:
    IXAudio2SourceVoice* m_pSourceVoice;
    VoiceCallback* m_pVoiceCallback;

    SoundChannelStates m_CurrentState;
    double m_TimePlaybackStarted;

public:
    SoundChannel();
    ~SoundChannel();

    void PlaySound(SoundObject* pSoundObject);
    void StopSound();

    IXAudio2SourceVoice* GetSourceVoice() { return m_pSourceVoice; }
    void SetSourceVoice(IXAudio2SourceVoice* voice, VoiceCallback* pVoiceCallback) { m_pSourceVoice = voice; m_pVoiceCallback = pVoiceCallback; }

    SoundChannelStates GetState() { return m_CurrentState; }
    void SetState(SoundChannelStates state) { m_CurrentState = state; }

    double GetTimePlaybackStarted() { return m_TimePlaybackStarted; }
};

class VoiceCallback : public IXAudio2VoiceCallback
{
public:
    SoundChannel* m_pChannel;

    VoiceCallback(SoundChannel* pChannel)
    {
        m_pChannel = pChannel;
    }

    virtual ~VoiceCallback()
    {
    }

    // Called when the voice has just finished playing a contiguous audio stream.
    void __stdcall OnStreamEnd()
    {
        m_pChannel->SetState( SoundChannel::SoundChannelState_Free );
    }

    // Unused methods are stubs.
    void __stdcall OnVoiceProcessingPassEnd() {}
    void __stdcall OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
    void __stdcall OnBufferEnd(void* pBufferContext) {}
    void __stdcall OnBufferStart(void* pBufferContext) {}
    void __stdcall OnLoopEnd(void* pBufferContext) {}
    void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) {}
};

class SoundPlayer
{
protected:
    static const int NUM_SOUNDOBJECTS_IN_POOL = 255;
    static const int MAX_CHANNELS = 10;

    IXAudio2* m_pEngine;
    IXAudio2MasteringVoice* m_pMasteringVoice;

    MySimplePool<SoundObject> m_SoundObjectPool;
    //TCPPListHead<SoundObject*> m_pSounds;
    SoundChannel* m_pChannels[MAX_CHANNELS];
    SoundObject m_Music;

#define SoundGroup_Music    0
#define SoundGroup_Effects  1

public:
    SoundPlayer();
    ~SoundPlayer();

    void OnFocusGained();
    void OnFocusLost();

    void PlayMusic(char* path);
    void PauseMusic();
    void UnpauseMusic();
    void StopMusic();

    //SoundObject* LoadSound(const char* path, const char* ext);
    SoundObject* LoadSound(const char* fullpath);
    SoundObject* LoadSound(MyFileObject* pFile);

    void Shutdown();

    int FindFreeChannel();
    int FindOldestChannel();

    int PlaySound(SoundObject* pSoundObject);
    int PlaySound(int soundid);
    void StopSound(int channel);
    void PauseSound(int channel);
    void ResumeSound(int channel);
    void PauseAll();
    void ResumeAll();
};

#endif //__SoundPlayerXAudio_H__
