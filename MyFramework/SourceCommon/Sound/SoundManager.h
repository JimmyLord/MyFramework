//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SoundManager_H__
#define __SoundManager_H__

static const int MAX_SOUND_CUE_NAME_LEN = 32;
static const int NUM_SOUND_CUES_TO_POOL = 128;

//struct SoundDefinition
//{
//    SoundObject* m_Sound;
//    char m_FullPath[MAX_PATH];
//
//    SoundDefinition()
//    {
//        m_Sound = 0;
//        m_FullPath[0] = 0;
//    }
//};

struct SoundCue : public CPPListNode
{
    char m_Name[MAX_SOUND_CUE_NAME_LEN];
    CPPListHead m_SoundObjects;

    SoundCue()
    {
        m_Name[0] = 0;
    }
};

class SoundManager
{
protected:
    MySimplePool<SoundCue> m_SoundCuePool;
    CPPListHead m_Cues;

public:
    SoundManager();
    ~SoundManager();

    SoundCue* CreateCue(const char* name);
    void AddSoundToCue(SoundCue* pCue, const char* fullpath);

    SoundCue* FindCueByName(const char* name);
    int PlayCueByName(const char* name);
};

#endif //__SoundManager_H__
