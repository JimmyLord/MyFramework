//
// Copyright (c) 2012-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "FileManager.h"
#include "../Helpers/MyFileObject.h"
#include "../Shaders/MyFileObjectShader.h"

#include "../../SourceWindows/SavedData.h" // TODO: Fix this dependency.
#if MYFW_NACL
#include "../../SourceNaCL/MainInstance.h"
#endif //MYFW_NACL

FileManager::FileManager(GameCore* pGameCore)
{
    m_pGameCore = pGameCore;

    // Create the pool of callback objects.
    if( m_pMyFileObject_FileFinishedLoadingCallbackPool.IsInitialized() == false )
        m_pMyFileObject_FileFinishedLoadingCallbackPool.AllocateObjects( CALLBACK_POOL_SIZE );

#if MYFW_WINDOWS
    GetCurrentDirectory( MAX_PATH, m_WorkingDirectory );
#else
    m_WorkingDirectory[0] = '\0';
#endif

#if USE_PTHREAD && !MYFW_NACL
    for( int threadIndex=0; threadIndex<1; threadIndex++ )
    {
        FileIOThreadObject* pThread = &m_Threads[threadIndex];

        pthread_mutex_init( &pThread->m_Mutex_FileLoading, nullptr );
        pthread_mutex_init( &pThread->m_Mutex_FileLists, nullptr );

        pthread_create( &pThread->m_FileIOThread, nullptr, &Thread_FileIO, this );

        pThread->m_KillFileIOThread = false;
    }
#endif //USE_PTHREAD && !MYFW_NACL
}

FileManager::~FileManager()
{
    PrintListOfOpenFiles();
    MyAssert( m_FilesLoaded.GetHead() == nullptr );
    MyAssert( m_FilesStillLoading.GetHead() == nullptr );

#if USE_PTHREAD && !MYFW_NACL
    for( int threadIndex=0; threadIndex<1; threadIndex++ )
    {
        FileIOThreadObject* pThread = &m_Threads[threadIndex];

        // Grab the thread mutex. There's a chance we have to wait for a file to finish loading.
        pthread_mutex_lock( &pThread->m_Mutex_FileLoading );

        // Wait for the thread to end.
        pThread->m_KillFileIOThread = true;
        pthread_mutex_unlock( &pThread->m_Mutex_FileLoading );
        pthread_join( pThread->m_FileIOThread, nullptr );
    }

    for( int threadIndex=0; threadIndex<1; threadIndex++ )
    {
        FileIOThreadObject* pThread = &m_Threads[threadIndex];

        pthread_mutex_destroy( &pThread->m_Mutex_FileLoading );
        pthread_mutex_destroy( &pThread->m_Mutex_FileLists );
    }
#endif //USE_PTHREAD && !MYFW_NACL

    m_pMyFileObject_FileFinishedLoadingCallbackPool.FreeObjects();
}

#if USE_PTHREAD && !MYFW_NACL
void* FileManager::Thread_FileIO(void* obj)
{
    FileManager* pthis = (FileManager*)obj;

    int threadIndex = 0; // TODO: Have filemanager pass in a proper threadid.
    FileIOThreadObject* pThread = &pthis->m_Threads[threadIndex];

    while( 1 )
    {
        pthread_mutex_lock( &pThread->m_Mutex_FileLists );
        MyFileObject* pFileLoading = pThread->m_FilesToLoad.GetHead();
        pthread_mutex_unlock( &pThread->m_Mutex_FileLists );

        if( pFileLoading )
        {
            pthread_mutex_lock( &pThread->m_Mutex_FileLoading );
        
            while( pFileLoading->m_FileLoadStatus == FileLoadStatus_Loading )
            {
                pFileLoading->Tick();
                //Sleep( 1000 );
            }

            pthread_mutex_lock( &pThread->m_Mutex_FileLists );
            pThread->m_FilesFinishedLoading.MoveTail( pFileLoading );
            pthread_mutex_unlock( &pThread->m_Mutex_FileLists );

            pthread_mutex_unlock( &pThread->m_Mutex_FileLoading );
        }

        if( pThread->m_KillFileIOThread )
            break;
    }

    return nullptr;
}
#endif //USE_PTHREAD && !MYFW_NACL

void FileManager::PrintListOfOpenFiles()
{
    LOGInfo( LOGTag, "Open Files:\n" );
    for( MyFileObject* pFile = m_FilesLoaded.GetHead(); pFile != nullptr; pFile = pFile->GetNext() )
    {
        LOGInfo( LOGTag, "   %s\n", pFile->GetFullPath() );
    }
    for( MyFileObject* pFile = m_FilesStillLoading.GetHead(); pFile != nullptr; pFile = pFile->GetNext() )
    {
        LOGInfo( LOGTag, "   %s\n", pFile->GetFullPath() );
    }
}

void FileManager::FreeFile(MyFileObject* pFile)
{
    MyAssert( pFile );
    pFile->Release(); // File's are refcounted, so release a reference to it.
}

unsigned int FileManager::CalculateTotalMemoryUsedByFiles()
{
    unsigned int totalSize = 0;

    for( MyFileObject* pFile = m_FilesLoaded.GetHead(); pFile != nullptr; pFile = pFile->GetNext() )
    {
        totalSize += pFile->m_FileLength;
    }

    for( MyFileObject* pFile = m_FilesStillLoading.GetHead(); pFile != nullptr; pFile = pFile->GetNext() )
    {
        totalSize += pFile->m_FileLength;
    }

    return totalSize;
}

MyFileObject* FileManager::CreateFileObject(const char* fullpath)
{
    MyFileObject* pFile = MyNew MyFileObject( this );

    pFile->ParseName( fullpath );

    m_FilesLoaded.AddTail( pFile );

    return pFile;
}

// Exposed to Lua, change elsewhere if function signature changes.
MyFileObject* FileManager::RequestFile(const char* filename)
{
    MyAssert( filename != nullptr );
    if( filename == nullptr )
        return nullptr;
    MyAssert( filename[0] != '\0' );
    if( filename[0] == '\0' )
        return nullptr;

    MyFileObject* pFile;

    // Check if the file has already been requested... might still be loading.
    pFile = FindFileByName( filename );
    if( pFile )
    {
        pFile->AddRef();
        return pFile;
    }

    // If the file wasn't already loaded create a new one and load it up.
    pFile = nullptr;

    int len = (int)strlen( filename );
    if( len > 5 && strcmp( &filename[len-5], ".glsl" ) == 0 )
        pFile = MyNew MyFileObjectShader( this );
    else
        pFile = MyNew MyFileObject( this );

    pFile->RequestFile( filename );

    m_FilesStillLoading.AddTail( pFile );

#if MYFW_NACL
    //LOGInfo( LOGTag, "Creating new NaCLFileObject\n" );

    NaCLFileObject* naclfile = MyNew NaCLFileObject( g_pInstance );
    naclfile->GetURL( filename );
    naclfile->m_pFile = pFile;

    pFile->m_pNaClFileObject = naclfile;
#endif //MYFW_NACL

    return pFile;
}

void FileManager::ReloadFile(MyFileObject* pFile)
{
    MyAssert( pFile );

    pFile->UnloadContents();
    m_FilesStillLoading.MoveTail( pFile );
}

MyFileObject* FileManager::FindFileByName(const char* filename)
{
    for( MyFileObject* pFile = m_FilesLoaded.GetHead(); pFile != nullptr; pFile = pFile->GetNext() )
    {
        if( strcmp( filename, pFile->GetFullPath() ) == 0 )
            return pFile;
    }

    for( MyFileObject* pFile = m_FilesStillLoading.GetHead(); pFile != nullptr; pFile = pFile->GetNext() )
    {
        if( strcmp( filename, pFile->GetFullPath() ) == 0 )
            return pFile;
    }

    return nullptr;
}

// Move file into loaded list, call finished loading callbacks, add file to memory panel in editor.
void FileManager::FinishSuccessfullyLoadingFile(MyFileObject* pFile)
{
    MyAssert( pFile->m_FileLoadStatus == FileLoadStatus_Success );

    m_FilesLoaded.MoveTail( pFile );

    // Inform all registered objects that the file finished loading.
    for( CPPListNode* pNode = pFile->m_FileFinishedLoadingCallbackList.GetHead(); pNode != nullptr; )
    {
        CPPListNode* pNextNode = pNode->GetNext();

        FileFinishedLoadingCallbackStruct* pCallbackStruct = (FileFinishedLoadingCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, pFile );

        pNode = pNextNode;
    }
}

void FileManager::Tick()
{
#if USE_PTHREAD && !MYFW_NACL
    int threadIndex = 0;
    FileIOThreadObject* pThread = &m_Threads[threadIndex];

    bool someFilesFinishedLoading = pThread->m_FilesFinishedLoading.GetHead() != nullptr;
    bool someFilesAreStillLoading = m_FilesStillLoading.GetHead() != nullptr;

    // Kick out early if all files are loaded.
    if( someFilesFinishedLoading == false && someFilesAreStillLoading == false )
        return;

    pthread_mutex_lock( &pThread->m_Mutex_FileLists );

    // Finish loading files.
    MyFileObject* pFile = pThread->m_FilesFinishedLoading.GetHead();

    while( pFile )
    {
        // pFile is now loaded by the thread, mark it as successfully loaded, will "FinishSuccessfullyLoadingFile" below.

        m_FilesStillLoading.MoveTail( pFile ); // Modifies thread variable.

        // Remove the ref that was preventing this file from being unloaded while in the load thread.
        pFile->Release(); // Prevent delete while loaded by thread.

        // If the file was successfully loaded by the other thread, then mark it loaded and call it's callbacks below.
        if( pFile->m_FileLoadStatus == FileLoadStatus_LoadedButNotFinalized )
        {
            pFile->m_FileLoadStatus = FileLoadStatus_Success;
        }

        // Get the next file from the list.
        pFile = pThread->m_FilesFinishedLoading.GetHead();
    }
#endif //USE_PTHREAD && !MYFW_NACL

    // Check if there are more files to load.
    {
        // Continue to tick any files still in the "loading" queue.
        MyFileObject* pNextFile;
        for( MyFileObject* pFile = m_FilesStillLoading.GetHead(); pFile != nullptr; pFile = pNextFile )
        {
			// Get the next file pointer.
            pNextFile = pFile->GetNext();

            //LOGInfo( LOGTag, "Loading File: %s\n", pFile->GetFullPath() );

            // If the file already failed to load, give up on it.
            //   In editor mode: we reset m_LoadFailed when focus regained and try all files again.
            if( pFile->m_FileLoadStatus > FileLoadStatus_Success )
            {
                continue;
            }

            // Add a ref to this file to prevent it from being deleted while in this loop.
            pFile->AddRef(); // Prevent delete during loop.

            // If we're done loading, move the file into the loaded list.
            if( pFile->m_FileLoadStatus == FileLoadStatus_Success )
            {
                //LOGInfo( LOGTag, "Finished loading: %s\n", pFile->GetFullPath() );

                // Move file into loaded list, call finished loading callbacks, add file to memory panel in editor.
                FinishSuccessfullyLoadingFile( pFile );
            }
            else
            {
#if USE_PTHREAD && !MYFW_NACL
                // Add a ref to prevent the file from being unloaded while in the load thread.
                pFile->AddRef(); // Prevent delete while loaded by thread.

                pThread->m_FilesToLoad.MoveTail( pFile ); // Modifies thread variable.
#else
#if !MYFW_NACL
                pFile->Tick();
#if !USE_PTHREAD
                pFile->m_FileLoadStatus = FileLoadStatus_Success;
#endif //!USE_PTHREAD
#endif //!MYFW_NACL
#endif //USE_PTHREAD && !MYFW_NACL
            }

            // Release the ref added to this file up above.
            pFile->Release(); // Prevent delete during loop.
        }
    }

#if USE_PTHREAD && !MYFW_NACL
    pthread_mutex_unlock( &pThread->m_Mutex_FileLists );
#endif
}

int FileManager::ReloadAnyUpdatedFiles(GameCore* pGameCore, FileManager_OnFileUpdated_CallbackFunction* pCallbackFunc)
{
    int numFilesUpdated = 0;

    MyFileObject* pNextFile;
    for( MyFileObject* pFile = m_FilesStillLoading.GetHead(); pFile != nullptr; pFile = pNextFile )
    {
        pNextFile = pFile->GetNext();

        bool updateavailable = pFile->IsNewVersionAvailable();

        if( updateavailable )
        {
            ReloadFile( pFile );
            pCallbackFunc( pGameCore, pFile );
            numFilesUpdated++;
        }
    }

    for( MyFileObject* pFile = m_FilesLoaded.GetHead(); pFile != nullptr; pFile = pNextFile )
    {
        pNextFile = pFile->GetNext();

        pFile->AddRef(); // Prevent file from being freed below.

        bool updateavailable = pFile->IsNewVersionAvailable();

        if( updateavailable )
        {
            ReloadFile( pFile );
            pCallbackFunc( pGameCore, pFile );
            numFilesUpdated++;
        }

        pFile->Release(); // Release ref from above.
    }

    return numFilesUpdated;
}

void FileManager::MoveFileToFrontOfFileLoadedList(MyFileObject* pFile)
{
    if( pFile == nullptr )
        return;

    // Make sure the file is in the loaded list.
    for( MyFileObject* pFileLoaded = m_FilesLoaded.GetHead(); pFileLoaded != nullptr; pFileLoaded = pFileLoaded->GetNext() )
    {
        if( pFileLoaded == pFile )
        {
            m_FilesLoaded.MoveHead( pFile );
            return;
        }
    }
}

#if MYFW_EDITOR
bool FileManager::DoesFileExist(const char* fullpath) // Static
{
#if MYFW_WINDOWS
    WIN32_FIND_DATAA data;
    memset( &data, 0, sizeof( data ) );

    void GetFileData(const char* path, WIN32_FIND_DATAA* data); // in MyFileObject.cpp

    GetFileData( fullpath, &data );

    if( data.dwFileAttributes != 0 )
        return true;
#else
    MyAssert( false );
#endif

    return false;
}

MyFileObject* FileManager::LoadFileNow(const char* fullpath)
{
    if( FileManager::DoesFileExist( fullpath ) == false )
        return nullptr;

    MyFileObject* pFile = RequestFile( fullpath );

    while( pFile->m_FileLoadStatus < FileLoadStatus_Success )
    {
        pFile->Tick();

        if( pFile->m_FileLoadStatus == FileLoadStatus_LoadedButNotFinalized )
            pFile->m_FileLoadStatus = FileLoadStatus_Success;
    }

    if( pFile->m_FileLoadStatus == FileLoadStatus_Success )
    {
        // Move file into loaded list, call finished loading callbacks, add file to memory panel in editor.
        FinishSuccessfullyLoadingFile( pFile );
    }

    return pFile;
}

void FileManager::ReloadFileNow(MyFileObject* pFile)
{
    MyAssert( pFile );
    MyAssert( FileManager::DoesFileExist( pFile->GetFullPath() ) == true );

    ReloadFile( pFile );

    while( pFile->m_FileLoadStatus < FileLoadStatus_Success )
    {
        pFile->Tick();

        if( pFile->m_FileLoadStatus == FileLoadStatus_LoadedButNotFinalized )
            pFile->m_FileLoadStatus = FileLoadStatus_Success;
    }

    if( pFile->m_FileLoadStatus == FileLoadStatus_Success )
    {
        // Move file into loaded list, call finished loading callbacks, add file to memory panel in editor.
        FinishSuccessfullyLoadingFile( pFile );
    }
}

void FileManager::RegisterFileUnloadedCallback(void* pObject, FileManager_Editor_OnFileUnloaded_CallbackFunction* pFunc)
{
    m_pFileUnloadedCallbackObj = pObject;
    m_pFileUnloadedCallbackFunc = pFunc;
}

void FileManager::Editor_UnloadFile(MyFileObject* pFile)
{
    if( m_pFileUnloadedCallbackFunc )
    {
        m_pFileUnloadedCallbackFunc( m_pFileUnloadedCallbackObj, pFile );
    }
}

void FileManager::RegisterFindAllReferencesCallback(void* pObject, FileManager_Editor_OnFindAllReferences_CallbackFunction* pFunc)
{
    m_pFindAllReferencesCallbackObj = pObject;
    m_pFindAllReferencesCallbackFunc = pFunc;
}

// Returns number of references.
int FileManager::Editor_FindAllReferences(MyFileObject* pFile)
{
    if( m_pFindAllReferencesCallbackFunc )
    {
        return m_pFindAllReferencesCallbackFunc( m_pFindAllReferencesCallbackObj, pFile );
    }

    return 0;
}

signed char FileSortFunc(CPPListNode* a, CPPListNode* b)
{
    MyFileObject* fa = (MyFileObject*)a;
    MyFileObject* fb = (MyFileObject*)b;

    int diff = strcmp( fa->GetExtensionWithDot(), fb->GetExtensionWithDot() );
    if( diff == 0 )
    {
        diff = strcmp( fa->GetFilenameWithoutExtension(), fb->GetFilenameWithoutExtension() );
    }

    return (signed char)diff;
}

void FileManager::SortFileLists()
{
    m_FilesLoaded.Sort( FileSortFunc );
}
#endif //MYFW_EDITOR

MySaveFileObject* CreatePlatformSpecificSaveFile()
{
    MySaveFileObject* pSaveFile = nullptr;

#if MYFW_WINDOWS
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_LINUX
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_WP8
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_ANDROID
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_NACL
    pSaveFile = MyNew MySaveFileObject();
#elif MYFW_BLACKBERRY
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_BADA
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_IOS
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_OSX
    pSaveFile = MyNew MySaveFileObject_FILE();
#elif MYFW_EMSCRIPTEN
    LOGError( LOGTag, "Save file not implemented\n" );
#endif

    return pSaveFile;
}

MySaveFileObject_FILE::MySaveFileObject_FILE()
{
    m_pFile = nullptr;
    m_pObjectToWriteBuffer = nullptr;
}

MySaveFileObject_FILE::~MySaveFileObject_FILE()
{
}

void MySaveFileObject_FILE::Reset()
{
    MySaveFileObject::Reset();

    if( m_pFile )
        fclose( m_pFile );
    m_pFile = nullptr;

    m_pObjectToWriteBuffer = nullptr;
}

void MySaveFileObject_FILE::WriteString(const char* path, const char* filename, const char* string)
{
    MyAssert( path != nullptr );
    MyAssert( filename != nullptr );
    MyAssert( string != nullptr );
    MyAssert( m_pFile == nullptr );

    if( m_pFile != nullptr )
        return;

    m_pFile = OpenSavedDataFile( path, filename, "wb" );
    m_pObjectToWriteBuffer = string;

    if( m_pFile )
    {
        MySaveFileObject::WriteString( path, filename, string );
    }
}

void MySaveFileObject_FILE::ReadString(const char* path, const char* filename)
{
    MyAssert( m_SaveFileOp == SFO_None );
    MyAssert( path != nullptr );
    MyAssert( filename != nullptr );
    MyAssert( m_pFile == nullptr );

    m_pFile = OpenSavedDataFile( path, filename, "rb" );

    if( m_pFile )
    {
        MySaveFileObject::ReadString( path, filename );
    }
    else
    {
        MyAssert( m_SaveFileOp == SFO_None );
        m_SaveFileOp = SFO_Read;
        m_OpComplete = true;
    }
}

void MySaveFileObject_FILE::Tick()
{
    if( m_SaveFileOp == SFO_None )
        return;
    if( m_OpComplete == true )
        return;

    MyAssert( m_pFile != nullptr );

    if( m_SaveFileOp == SFO_Read )
    {
        fseek( m_pFile, 0, SEEK_END );
        long length = ftell( m_pFile );
        if( length > 0 )
        {
            fseek( m_pFile, 0, SEEK_SET );
            m_pReadStringBuffer = MyNew char[length+1];

            fread( m_pReadStringBuffer, length, 1, m_pFile );
            m_pReadStringBuffer[length] = '\0';
        }

        fclose( m_pFile );
        m_pFile = nullptr;
        m_OpComplete = true;
    }

    if( m_SaveFileOp == SFO_Write )
    {
        MyAssert( m_pObjectToWriteBuffer != nullptr );

        int length = (int)strlen( m_pObjectToWriteBuffer );
        fwrite( m_pObjectToWriteBuffer, length, 1, m_pFile );
        fclose( m_pFile );
        m_pFile = nullptr;
        m_OpComplete = true;
    }
}
