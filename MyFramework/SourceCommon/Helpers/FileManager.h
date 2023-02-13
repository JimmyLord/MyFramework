//
// Copyright (c) 2012-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __FileManager_H__
#define __FileManager_H__

#include "../DataTypes/MyActivePool.h"

class FileManager;
class GameCore;
class MyFileObject;
class MySaveFileObject;
class TextureDefinition;

MySaveFileObject* CreatePlatformSpecificSaveFile();

typedef void FileManager_OnFileUpdated_CallbackFunction(GameCore* pGameCore, MyFileObject* pFile);
typedef void FileManager_Editor_OnFileUnloaded_CallbackFunction(void* pObject, MyFileObject* pFile);
typedef int FileManager_Editor_OnFindAllReferences_CallbackFunction(void* pObject, MyFileObject* pFile);

typedef void FileFinishedLoadingCallbackFunc(void* pObjectPtr, MyFileObject* pFile);
struct FileFinishedLoadingCallbackStruct : public TCPPListNode<FileFinishedLoadingCallbackStruct*>
{
    void* pObj;
    FileFinishedLoadingCallbackFunc* pFunc;
};

class FileManager
{
    struct FileIOThreadObject
    {
#if USE_PTHREAD
        // Thread object.
        pthread_t m_FileIOThread;

        // General thread control.  The mutex will be owned by the thread when it's loading a file.
        pthread_mutex_t m_Mutex_FileLoading;
        bool m_KillFileIOThread;

        // Lists of files used to communicate between main thread and file io thread.
        pthread_mutex_t m_Mutex_FileLists;
        TCPPListHead<MyFileObject*> m_FilesToLoad;
        TCPPListHead<MyFileObject*> m_FilesFinishedLoading;
#endif //USE_PTHREAD
    };

protected:
    GameCore* m_pGameCore;

    TCPPListHead<MyFileObject*> m_FilesLoaded;
    TCPPListHead<MyFileObject*> m_FilesStillLoading;

    char m_WorkingDirectory[MAX_PATH];

    static const int CALLBACK_POOL_SIZE = 1000;
    MySimplePool<FileFinishedLoadingCallbackStruct> m_pMyFileObject_FileFinishedLoadingCallbackPool;

protected:
#if USE_PTHREAD
    FileIOThreadObject m_Threads[1]; // TODO: There should be one of these for each file system in use.

    static void* Thread_FileIO(void* obj);
#endif //USE_PTHREAD

public:
    FileManager(GameCore* pGameCore);
    virtual ~FileManager();

    // Getters.
    GameCore* GetGameCore() { return m_pGameCore; }
    MySimplePool<FileFinishedLoadingCallbackStruct>* GetFileFinishedLoadingCallbackPool() { return &m_pMyFileObject_FileFinishedLoadingCallbackPool; }

    void PrintListOfOpenFiles();
    void FreeFile(MyFileObject* pFile);
    //void FreeAllFiles();

    unsigned int CalculateTotalMemoryUsedByFiles();

    const char* GetWorkingDirectory() { return m_WorkingDirectory; }

    MyFileObject* CreateFileObject(const char* fullpath);
    virtual MyFileObject* RequestFile(const char* filename); // Will add a ref.
    MyFileObject* FindFileByName(const char* filename); // Doesn't add a ref to the file.
    void ReloadFile(MyFileObject* pFile);
    void FinishSuccessfullyLoadingFile(MyFileObject* pFile);
    void Tick();
    int ReloadAnyUpdatedFiles(GameCore* pGameCore, FileManager_OnFileUpdated_CallbackFunction* pCallbackFunc);

    MyFileObject* GetFirstFileLoaded() { return m_FilesLoaded.GetHead(); }
    MyFileObject* GetFirstFileStillLoading() { return m_FilesStillLoading.GetHead(); }

    void MoveFileToFrontOfFileLoadedList(MyFileObject* pFile);

#if MYFW_EDITOR
protected:
    void* m_pFileUnloadedCallbackObj;
    FileManager_Editor_OnFileUnloaded_CallbackFunction* m_pFileUnloadedCallbackFunc;

    void* m_pFindAllReferencesCallbackObj;
    FileManager_Editor_OnFindAllReferences_CallbackFunction* m_pFindAllReferencesCallbackFunc;    

public:
    static bool DoesFileExist(const char* fullpath);
    MyFileObject* LoadFileNow(const char* fullpath);
    void ReloadFileNow(MyFileObject* pFile);

    void RegisterFileUnloadedCallback(void* pObject, FileManager_Editor_OnFileUnloaded_CallbackFunction* pFunc);
    void Editor_UnloadFile(MyFileObject* pFile);

    void RegisterFindAllReferencesCallback(void* pObject, FileManager_Editor_OnFindAllReferences_CallbackFunction* pFunc);
    int Editor_FindAllReferences(MyFileObject* pFile);

    void SortFileLists();
#endif //MYFW_EDITOR
};

enum SaveFileOperation
{
    SFO_None,
    SFO_Read,
    SFO_Write,
};

class MySaveFileObject
{
public:
    SaveFileOperation m_SaveFileOp;
    bool m_OpComplete;
    char* m_pReadStringBuffer; // Allocated by this object(or subclasses), so needs to be cleaned up.

public:
    MySaveFileObject()
    {
        m_SaveFileOp = SFO_None;
        m_OpComplete = false;
        m_pReadStringBuffer = 0;
    }
    virtual ~MySaveFileObject() {}

    virtual void Reset()
    {
        m_SaveFileOp = SFO_None;
        m_OpComplete = false;
        SAFE_DELETE_ARRAY( m_pReadStringBuffer );
    }

    virtual void WriteString(const char* path, const char* filename, const char* string)
    {
        MyAssert( m_SaveFileOp == SFO_None );
        m_SaveFileOp = SFO_Write;
        m_OpComplete = false;
    }

    virtual void ReadString(const char* path, const char* filename)
    {
        MyAssert( m_SaveFileOp == SFO_None );
        m_SaveFileOp = SFO_Read;
        m_OpComplete = false;
    }

    virtual void Tick()
    {
        m_OpComplete = true;
    }

    virtual bool QueryIsDone()
    {
        return m_OpComplete || m_SaveFileOp == SFO_None;
    }

    virtual char* QueryString()
    {
        return m_pReadStringBuffer;
    }
};

class MySaveFileObject_FILE : public MySaveFileObject
{
public:
    FILE* m_pFile;
    const char* m_pObjectToWriteBuffer; // Managed by caller to this function... i.e. will not be deleted when write is complete.

public:
    MySaveFileObject_FILE();
    ~MySaveFileObject_FILE();

    virtual void Reset();

    virtual void WriteString(const char* path, const char* filename, const char* string);
    virtual void ReadString(const char* path, const char* filename);
    virtual void Tick();

    //virtual bool QueryIsDone();
    //virtual char* QueryString();
};

#endif //__FileManager_H__
