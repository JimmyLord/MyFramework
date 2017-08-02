//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyFileObject_H__
#define __MyFileObject_H__

class MyFileObject;

#if MYFW_USING_WX
char* PlatformSpecific_LoadFile(const char* filename, int* length = 0, const char* file = __FILE__, unsigned long line = __LINE__);
#endif

typedef void (*PanelObjectListObjectCallback)(void*);

typedef void (*FileFinishedLoadingCallbackFunc)(void* pObjectPtr, MyFileObject* pFile);
struct FileFinishedLoadingCallbackStruct : public CPPListNode
{
    void* pObj;
    FileFinishedLoadingCallbackFunc pFunc;
};

extern MySimplePool<FileFinishedLoadingCallbackStruct> g_pMyFileObject_FileFinishedLoadingCallbackPool;

#if MYFW_NACL
class NaCLFileObject;
#endif

enum FileLoadStatus
{
    FileLoadStatus_Loading,
    FileLoadStatus_LoadedButNotFinalized,

    // all states above need to be loading
    FileLoadStatus_Success,
    // all states below need to be errors

    FileLoadStatus_Error_FileNotFound,
    FileLoadStatus_Error_Other,
};

class MyFileObject : public CPPListNode, public RefCount
#if MYFW_USING_WX
, public wxEvtHandler
#endif
{
    friend class FileManager;

    static const int CALLBACK_POOL_SIZE = 1000;

protected:
    CPPListHead m_FileFinishedLoadingCallbackList;

    char* m_FullPath;
    char* m_FilenameWithoutExtension;
    char* m_ExtensionWithDot; // will be "." if no extension
    FileLoadStatus m_FileLoadStatus;
    unsigned int m_FileLength;
    char* m_pBuffer;
    int m_BytesRead;

#if MYFW_NACL
    NaCLFileObject* m_pNaClFileObject;
#endif

#if MYFW_WINDOWS
    FILETIME m_FileLastWriteTime;
#endif

public:
    MyFileObject();
    virtual ~MyFileObject();
    SetClassnameBase( "MyFileObject" ); // only first 8 character count.

    void GenerateNewFullPathFilenameInSameFolder(char* newfilename, char* buffer, int buffersize);
    void GenerateNewFullPathExtensionWithSameNameInSameFolder(const char* newextension, char* buffer, int buffersize);
    const char* GetNameOfDeepestFolderPath();

    const char* Rename(const char* newnamewithoutextension);

    bool IsFinishedLoading();
    const char* GetFullPath() { return m_FullPath; }
    const char* GetFilenameWithoutExtension() { return m_FilenameWithoutExtension; }
    const char* GetExtensionWithDot() { return m_ExtensionWithDot; } // will be "." if no extension
    FileLoadStatus GetFileLoadStatus() { return m_FileLoadStatus; }
    unsigned int GetFileLength() { return m_FileLength; }
    const char* GetBuffer() { return m_pBuffer; }

#if MYFW_WINDOWS
    FILETIME GetFileLastWriteTime() { return m_FileLastWriteTime; }
#endif

    // Callbacks
    void RegisterFileFinishedLoadingCallback(void* pObj, FileFinishedLoadingCallbackFunc pCallback);
    void UnregisterFileFinishedLoadingCallback(void* pObj);

protected:
    void RequestFile(const char* filename);
    void ParseName(const char* filename);

    void Tick();

    virtual void UnloadContents();

    bool IsNewVersionAvailable();

public:
    void FakeFileLoad(char* buffer, int length);

    void UpdateTimestamp();

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_ViewInWatchWindow = 1000,
        RightClick_OpenFile,
        RightClick_OpenContainingFolder,
    };

    void OSLaunchFile(bool createfileifdoesntexist);
    void OSOpenContainingFolder();

    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((MyFileObject*)pObjectPtr)->OnLeftClick( count ); }
    void OnLeftClick(unsigned int count);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((MyFileObject*)pObjectPtr)->OnRightClick(); }
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((MyFileObject*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    PanelObjectListObjectCallback m_CustomLeftClickCallback;
    void* m_CustomLeftClickObject;
    void SetCustomLeftClickCallback(PanelObjectListObjectCallback callback, void* object);
#endif //MYFW_USING_WX
};

#endif //__MyFileObject_H__
