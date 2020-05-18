//
// Copyright (c) 2012-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyFileObject_H__
#define __MyFileObject_H__

#include "FileManager.h"

class MyFileObject;

#if MYFW_WINDOWS
FileTimeStamp GetFileLastModifiedTime(const char* path);
#endif

#if MYFW_EDITOR
char* PlatformSpecific_LoadFile(const char* relativePath, int* length = nullptr, const char* file = __FILE__, unsigned long line = __LINE__);
#endif

typedef void PanelObjectListObjectCallback(void*);

#if MYFW_NACL
class NaCLFileObject;
#endif

enum FileLoadStatus
{
    FileLoadStatus_Loading,
    FileLoadStatus_LoadedButNotFinalized,

    // All states above need to be loading.
    FileLoadStatus_Success,
    // All states below need to be errors.

    FileLoadStatus_Error_FileNotFound,
    FileLoadStatus_Error_Other,
};

class MyFileObject : public TCPPListNode<MyFileObject*>, public RefCount
{
private:
    friend class FileManager;
    friend class NaCLFileObject;

protected:
    FileManager* m_pFileManager;

    TCPPListHead<FileFinishedLoadingCallbackStruct*> m_FileFinishedLoadingCallbackList;

    char* m_FullPath; // Path relative to the working directory.
    char* m_FilenameWithoutExtension;
    char* m_ExtensionWithDot; // Will be "." if no extension.
    FileLoadStatus m_FileLoadStatus;
    unsigned int m_FileLength;
    char* m_pBuffer;
    int m_BytesRead;

#if MYFW_NACL
    NaCLFileObject* m_pNaClFileObject;
#endif

#if MYFW_WINDOWS
    FILETIME m_FileLastWriteTime;
#else
    time_t m_FileLastWriteTime;
#endif

public:
    MyFileObject(FileManager* pFileManager);
    virtual ~MyFileObject();
    SetClassnameBase( "MyFileObject" ); // Only first 8 character count.

    // These 2 methods are called by the GameCore at startup and shutdown.
    static void SystemStartup();
    static void SystemShutdown();

    void GenerateNewFullPathFilenameInSameFolder(char* newFilename, char* buffer, int bufferSize);
    void GenerateNewFullPathExtensionWithSameNameInSameFolder(const char* newExtension, char* buffer, int bufferSize);
    const char* GetNameOfDeepestFolderPath();

    const char* Rename(const char* newNameWithoutExtension);

    bool IsFinishedLoading();
    const char* GetFullPath() const { return m_FullPath; }
    const char* GetFilename();
    const char* GetFilenameWithoutExtension() { return m_FilenameWithoutExtension; }
    const char* GetExtensionWithDot() { return m_ExtensionWithDot; } // Will be "." if no extension.
    FileLoadStatus GetFileLoadStatus() { return m_FileLoadStatus; }
    unsigned int GetFileLength() { return m_FileLength; }
    const char* GetBuffer() { return m_pBuffer; }

#if MYFW_NACL
    void SetNaClFileObject(NaCLFileObject* pNaClFile) { m_pNaClFileObject = pNaClFile; }
    NaCLFileObject* GetNaClFileObject() { return m_pNaClFileObject; }
#endif

#if MYFW_WINDOWS
    FILETIME GetFileLastWriteTime() { return m_FileLastWriteTime; }
#else
    time_t GetFileLastWriteTime() { return m_FileLastWriteTime; }
#endif

    // Callbacks.
    void RegisterFileFinishedLoadingCallback(void* pObj, FileFinishedLoadingCallbackFunc* pCallback);
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
#if MYFW_EDITOR
    enum RightClickOptions
    {
        RightClick_ViewInWatchWindow = 1000,
        RightClick_OpenFile,
        RightClick_OpenContainingFolder,
        RightClick_UnloadFile,
        RightClick_FindAllReferences,
    };

    bool m_ShowInMemoryPanel;
    void MemoryPanel_Hide() { m_ShowInMemoryPanel = false; }
    bool MemoryPanel_IsVisible() { return m_ShowInMemoryPanel; }

    void OSLaunchFile(bool createFileIfDoesntExist);
    void OSOpenContainingFolder();

    void OnPopupClick(FileManager* pFileManager, MyFileObject* pFileObject, int id);
#endif //MYFW_EDITOR
};

#endif //__MyFileObject_H__
