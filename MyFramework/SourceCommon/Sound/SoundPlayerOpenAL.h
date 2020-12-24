//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SoundPlayer_H__
#define __SoundPlayer_H__

#if !MYFW_ANDROID

#include "DataTypes/MyActivePool.h"

#if MYFW_IOS || MYFW_OSX
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
	#if MYFW_IOS
	#include <OpenAL/oalStaticBufferExtension.h>
	#endif //MYFW_IOS
#define USE_ALUT    0
#elif MYFW_WINDOWS
#include <al.h>
#include <alc.h>
#define USE_ALUT    0
#else
#include <AL/al.h>
#include <AL/alc.h>
//#include <AL/alut.h>
#define USE_ALUT    0
#endif

#define MAX_BUFFERS 100
#define NUM_SOURCES 100

bool CheckForOpenALErrors(const char* description);
bool CheckForOpenALCErrors(ALCdevice* pDevice, const char* description);
#if USE_ALUT
bool CheckForALUTErrors(const char* description);
#endif

struct SoundObject : public TCPPListNode<SoundObject*>, public RefCount
{
public:
    char m_FullPath[MAX_PATH];
    ALuint m_Sound;

    MySimplePool<SoundObject>* m_pSourcePool;

public:
    SoundObject();
    virtual void Release() override; // From RefCount.

    cJSON* ExportAsJSONObject();
    const char* GetFullPath() { return m_FullPath; }
};

class SoundPlayer
{
protected:
    ALuint m_Buffers[MAX_BUFFERS];
    SoundObject m_Sources[NUM_SOURCES];

    int m_NextID;

#if !USE_ALUT
    ALCdevice* m_pDevice;
    ALCcontext* m_pContext;
#endif

public:
    SoundPlayer();
    ~SoundPlayer();

    void ActivateSoundContext();
    void DeactivateSoundContext();

    void OnFocusGained();
    void OnFocusLost();

    SoundObject* LoadSound(const char* buffer, unsigned int buffersize);
    SoundObject* LoadSound(const char* fullpath);
    void Shutdown();
    int PlaySound(SoundObject* pSoundObject);
    void StopSound(int soundid);
    void PauseSound(int soundid);
    void ResumeSound(int soundid);
    void PauseAll();
    void ResumeAll();
};

#endif //!MYFW_ANDROID

#endif //__SoundPlayer_H__
