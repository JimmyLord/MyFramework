//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SoundPlayer_H__
#define __SoundPlayer_H__

struct Mix_Chunk;

struct SoundObject : public CPPListNode
{
    Mix_Chunk* m_Sound;

    SoundObject()
    {
        m_Sound = 0;
    }
};

class SoundPlayer
{
protected:
#define MAX_SOUNDS 255

    //MySimplePool<SoundObject> m_SoundObjectPool;
    SoundObject m_Sounds[MAX_SOUNDS];
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

    SoundObject* LoadSound(const char* path, const char* ext);
    SoundObject* LoadSound(const char* fullpath);
    void Shutdown();
    int PlaySound(SoundObject* pSoundObject);
    int PlaySound(int soundid);
    void StopSound(int channel);
    void PauseSound(int channel);
    void ResumeSound(int channel);
    void PauseAll();
    void ResumeAll();
};

#endif //__SoundPlayer_H__
