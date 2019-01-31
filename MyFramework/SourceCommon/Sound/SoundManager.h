//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SoundManager_H__
#define __SoundManager_H__

#include "../DataTypes/MyActivePool.h"

class MyFileObject;
class SoundCue;
class SoundManager;
class SoundObject;

static const int MAX_SOUND_CUE_NAME_LEN = 32;
static const int NUM_SOUND_CUES_TO_POOL = 128;

typedef void SoundCueCallbackFunc(void* pObjectPtr, SoundCue* pSoundCue);
struct SoundCueCallbackStruct
{
    void* pObj;
    SoundCueCallbackFunc* pFunc;
};

class SoundCue : public TCPPListNode<SoundCue*>, public RefCount
{
    friend class SoundManager;

protected:
    bool m_FullyLoaded;
    bool m_UnsavedChanges;

    char m_Name[MAX_SOUND_CUE_NAME_LEN]; // If [0] == '\0', cue won't save to disk.
    MyFileObject* m_pFile;
    MySimplePool<SoundCue>* m_pSourcePool;

#if MYFW_EDITOR
    std::vector<SoundObject*> m_pSoundObjects;
#else
    MyList<SoundObject*> m_pSoundObjects;
#endif

public:
    SoundCue();
    virtual ~SoundCue();

    virtual void Release() override; // Override from RefCount.

    void ImportFromFile();

    MyFileObject* GetFile() { return m_pFile; }

    void SetName(const char* name);
    const char* GetName() { return m_Name; }

    bool IsFullyLoaded() { return m_FullyLoaded; }

#if MYFW_EDITOR
public:
    void SaveSoundCue(const char* relativefolder);
#endif //MYFW_EDITOR
};

class SoundManager
{
    static const int MAX_REGISTERED_CALLBACKS = 1; // TODO: Fix this hardcodedness.

protected:
    MySimplePool<SoundCue> m_SoundCuePool;
    TCPPListHead<SoundCue*> m_Cues;
    TCPPListHead<SoundCue*> m_CuesStillLoading;

    MyList<SoundCueCallbackStruct> m_pSoundCueCreatedCallbackList;
    MyList<SoundCueCallbackStruct> m_pSoundCueUnloadedCallbackList;

protected:
    SoundCue* GetCueFromPool();

public:
    SoundManager();
    ~SoundManager();

    void Tick();

    SoundCue* CreateCue(const char* name);
    SoundCue* LoadCue(const char* fullpath);
    SoundCue* LoadExistingCue(SoundCue* pCue);
    void UnloadCue(SoundCue* pCue);
    void AddSoundToCue(SoundCue* pCue, const char* fullpath);
    void RemoveSoundFromCue(SoundCue* pCue, SoundObject* pSoundObject);

    SoundCue* FindCueByName(const char* name);
    SoundCue* FindCueByFilename(const char* fullpath);
    int PlayCueByName(const char* name);

    int PlayCue(SoundCue* pCue);

    // Getters.
    SoundCue* GetCues() { return (SoundCue*)m_Cues.GetHead(); }
    SoundCue* GetCuesStillLoading() { return (SoundCue*)m_CuesStillLoading.GetHead(); }

    // Callbacks.
    void RegisterSoundCueCreatedCallback(void* pObj, SoundCueCallbackFunc* pCallback);
    void RegisterSoundCueUnloadedCallback(void* pObj, SoundCueCallbackFunc* pCallback);

#if MYFW_EDITOR
protected:
    unsigned int m_NumRefsPlacedOnSoundCueBySystem; // Stores the lowest refcount required to unload a soundcue in editor.

public:
    void SaveAllCues(bool saveunchanged = false);
    void Editor_AddToNumRefsPlacedOnSoundCueBySystem() { m_NumRefsPlacedOnSoundCueBySystem += 1; }
    unsigned int Editor_GetNumRefsPlacedOnSoundCueBySystem() { return m_NumRefsPlacedOnSoundCueBySystem; }

public:
    void AddSoundCueToMemoryPanel(SoundCue* pCue);
#endif //MYFW_EDITOR
};

#endif //__SoundManager_H__
