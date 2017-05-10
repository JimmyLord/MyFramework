//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
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

class SoundManager;
class SoundCue;

typedef void (*SoundCueCallbackFunc)(void* pObjectPtr, SoundCue* pSoundCue);
struct SoundCueCallbackStruct
{
    void* pObj;
    SoundCueCallbackFunc pFunc;
};

#if MYFW_USING_WX
class SoundCueWxEventHandler : public wxEvtHandler
{
public:
    enum RightClickOptions
    {
        RightClick_Rename = 1000,
        RightClick_Unload = 1001,
    };

public:
    SoundCue* m_pSoundCue;
    SoundObject* m_pSoundObject;

public:
    SoundCueWxEventHandler()
    {
        m_pSoundCue = 0;
        m_pSoundObject = 0;
    };
    void OnPopupClick(wxEvent &evt);
};
#endif

class SoundCue : public CPPListNode, public RefCount
{
public:
    bool m_FullyLoaded;

    char m_Name[MAX_SOUND_CUE_NAME_LEN]; // if [0] == 0, cue won't save to disk.
    MyFileObject* m_pFile;
    MySimplePool<SoundCue>* m_pSourcePool;

#if MYFW_USING_WX
    std::vector<SoundObject*> m_pSoundObjects;
#else
    MyList<SoundObject*> m_pSoundObjects;
#endif

public:
    SoundCue();
    virtual ~SoundCue();

    virtual void Release(); // override from RefCount

    void ImportFromFile();

    void SetName(const char* name);

public:
#if MYFW_USING_WX
    void SaveSoundCue(const char* relativefolder);

    SoundCueWxEventHandler m_WxEventHandler;
    wxTreeItemId m_TreeIDRightClicked;

    static void StaticOnLabelEdit(void* pObjectPtr, wxTreeItemId id, wxString newlabel) { ((SoundCue*)pObjectPtr)->OnLabelEdit( newlabel ); }
    void OnLabelEdit(wxString newlabel);

    static void StaticOnDrag(void* pObjectPtr) { ((SoundCue*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int index) { ((SoundCue*)pObjectPtr)->OnLeftClick( index ); }
    void OnLeftClick(unsigned int count);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((SoundCue*)pObjectPtr)->OnRightClick( id ); }
    void OnRightClick(wxTreeItemId treeid);
#endif //MYFW_USING_WX
};

#if MYFW_USING_WX
class SoundManagerWxEventHandler : public wxEvtHandler
{
public:
    enum RightClickOptions
    {
        RightClick_LoadSoundFile = 1000,
        RightClick_CreateNewCue,
        RightClick_RemoveSoundObjectFromCue,
    };

public:
    SoundManager* m_pSoundManager;
    SoundCue* m_pSoundCue;
    SoundObject* m_pSoundObject;

public:
    SoundManagerWxEventHandler()
    {
        m_pSoundManager = 0;
        m_pSoundCue = 0;
        m_pSoundObject = 0;
    };
    void OnPopupClick(wxEvent &evt);
};
#endif

class SoundManager
{
    static const int MAX_REGISTERED_CALLBACKS = 1; // TODO: fix this hardcodedness

#if MYFW_USING_WX
    friend class SoundManagerWxEventHandler;
#endif

protected:
    MySimplePool<SoundCue> m_SoundCuePool;
    CPPListHead m_Cues;
    CPPListHead m_CuesStillLoading;

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
    void UnloadCue(SoundCue* pCue);
    void AddSoundToCue(SoundCue* pCue, const char* fullpath);
    void RemoveSoundFromCue(SoundCue* pCue, SoundObject* pSoundObject);

    SoundCue* FindCueByName(const char* name);
    SoundCue* FindCueByFilename(const char* fullpath);
    int PlayCueByName(const char* name);

    int PlayCue(SoundCue* pCue);

    // Callbacks
    void RegisterSoundCueCreatedCallback(void* pObj, SoundCueCallbackFunc pCallback);
    void RegisterSoundCueUnloadedCallback(void* pObj, SoundCueCallbackFunc pCallback);

#if MYFW_USING_WX
    void SaveAllCues(bool saveunchanged = false);

    SoundManagerWxEventHandler m_WxEventHandler;
    wxTreeItemId m_TreeIDRightClicked;

    // Callbacks for root of tree
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int index) { ((SoundManager*)pObjectPtr)->OnLeftClick( index ); }
    void OnLeftClick(unsigned int count);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((SoundManager*)pObjectPtr)->OnRightClick( id ); }
    void OnRightClick(wxTreeItemId treeid);

    // Callbacks for sound object
    static void StaticOnLeftClickSoundObject(void* pObjectPtr, wxTreeItemId id, unsigned int index) { ((SoundManager*)pObjectPtr)->OnLeftClickSoundObject( index ); }
    void OnLeftClickSoundObject(unsigned int count);

    static void StaticOnRightClickSoundObject(void* pObjectPtr, wxTreeItemId id) { ((SoundManager*)pObjectPtr)->OnRightClickSoundObject( id ); }
    void OnRightClickSoundObject(wxTreeItemId treeid);
#endif
};

#endif //__SoundManager_H__
