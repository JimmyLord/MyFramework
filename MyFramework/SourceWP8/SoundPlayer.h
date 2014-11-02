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

#ifndef __SoundPlayer_H__
#define __SoundPlayer_H__

#include <xaudio2.h>
#include "../SourceCommon/Sound/WaveLoader.h"
#include <mfmediaengine.h>

#define MAX_AUDIO_FILES             100
#define MAX_CONCURRENT_SOUNDS       16

class SoundPlayer
{
protected:
    IXAudio2* m_pAudioEngine;
    IXAudio2MasteringVoice* m_pMasterVoiceEffects;
    //IXAudio2MasteringVoice* m_pMasterVoiceMusic;
    Microsoft::WRL::ComPtr<IMFMediaEngine> m_mediaEngine;


    MyWaveDescriptor m_WaveDescriptors[MAX_AUDIO_FILES];
    unsigned int m_NumAudioBuffersLoaded;

    IXAudio2SourceVoice* m_pSourceVoice[MAX_CONCURRENT_SOUNDS];

public:
    SoundPlayer();
    ~SoundPlayer();

    void OnFocusGained();
    void OnFocusLost();

    void PlayMusic(char* path);
    void PauseMusic();
    void UnpauseMusic();
    void StopMusic();

    int LoadSound(const char* buffer, unsigned int buffersize);
    //int LoadSound(const char* path, const char* ext);
    void Shutdown();
    void PlaySound(int soundid, bool looping = false);
    void StopSound(int soundid);
    void PauseSound(int soundid);
    void ResumeSound(int soundid);
    void PauseAll();
    void ResumeAll();
};

#endif //__SoundPlayer_H__
