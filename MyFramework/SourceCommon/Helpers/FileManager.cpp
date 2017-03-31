//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "FileManager.h"
#if MYFW_NACL
#include "../../SourceNaCL/MainInstance.h"
#endif //MYFW_NACL

#pragma warning( push )
#pragma warning( disable : 4996 )
#include "../../Libraries/LodePNG/lodepng.h"
#pragma warning( pop )

FileManager* g_pFileManager = 0;

FileManager::FileManager()
{
#if USE_PTHREAD && !MYFW_NACL
    for( int threadid=0; threadid<1; threadid++ )
    {
        pthread_mutex_init( &m_FileIOThreadLocks[threadid], 0 );
        pthread_mutex_lock( &m_FileIOThreadLocks[threadid] );
        m_FileIOThreadIsLocked[threadid] = true;

        pthread_create( &m_FileIOThreads[threadid], 0, &Thread_FileIO, this );

        m_KillFileIOThread[threadid] = 0;
        m_pLastFileLoadedByThread[threadid] = 0;
        m_pFileThisFileIOThreadIsLoading[threadid] = 0;
    }
#endif //USE_PTHREAD && !MYFW_NACL
}

FileManager::~FileManager()
{
    PrintListOfOpenFiles();
    MyAssert( m_FilesLoaded.GetHead() == 0 );
    MyAssert( m_FilesStillLoading.GetHead() == 0 );
    //FreeAllFiles();

#if USE_PTHREAD && !MYFW_NACL
    // grab the thread mutex... small chance we have to wait for a file to finish loading.
    for( int threadid=0; threadid<1; threadid++ )
    {
        if( m_FileIOThreadIsLocked[threadid] == false )
            pthread_mutex_lock( &m_FileIOThreadLocks[threadid] );

        // wait for the thread to end.
        m_KillFileIOThread[threadid] = true;
        pthread_mutex_unlock( &m_FileIOThreadLocks[threadid] );
        pthread_join( m_FileIOThreads[threadid], 0 );
    }
#endif //USE_PTHREAD && !MYFW_NACL

    for( int threadid=0; threadid<1; threadid++ )
    {
        pthread_mutex_destroy( &m_FileIOThreadLocks[threadid] );
    }
}

#if USE_PTHREAD && !MYFW_NACL
void* FileManager::Thread_FileIO(void* obj)
{
    FileManager* pthis = (FileManager*)obj;

    int threadid = 0; // TODO: have filemanager pass in a proper threadid.

    while( 1 )
    {
        pthread_mutex_lock( &pthis->m_FileIOThreadLocks[threadid] );

        if( pthis->m_pFileThisFileIOThreadIsLoading[threadid] )
        {
            while( pthis->m_pFileThisFileIOThreadIsLoading[threadid]->m_FileLoadStatus == FileLoadStatus_Loading )
            {
                pthis->m_pFileThisFileIOThreadIsLoading[threadid]->Tick();
            }

            pthis->m_pFileThisFileIOThreadIsLoading[threadid] = 0;
        }

        pthread_mutex_unlock( &pthis->m_FileIOThreadLocks[threadid] );

        if( pthis->m_KillFileIOThread[threadid] )
            break;
    }

    return 0;
}
#endif //USE_PTHREAD && !MYFW_NACL

void FileManager::PrintListOfOpenFiles()
{
    LOGInfo( LOGTag, "Open Files:\n" );
    for( CPPListNode* pNode = m_FilesLoaded.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        MyFileObject* pFile = (MyFileObject*)pNode;
        LOGInfo( LOGTag, "   %s\n", pFile->m_FullPath );
    }
    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        MyFileObject* pFile = (MyFileObject*)pNode;
        LOGInfo( LOGTag, "   %s\n", pFile->m_FullPath );
    }
}

void FileManager::FreeFile(MyFileObject* pFile)
{
    MyAssert( pFile );
    pFile->Release(); // file's are refcounted, so release a reference to it.
}

//void FileManager::FreeAllFiles()
//{
//    // TODO: remove this function... why did I put this here..
//    MyAssert( m_FilesLoaded.GetHead() == 0 );
//    MyAssert( m_FilesStillLoading.GetHead() == 0 );
//}

unsigned int FileManager::CalculateTotalMemoryUsedByFiles()
{
    unsigned int totalsize = 0;

    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_FilesLoaded.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileObject* pFile = (MyFileObject*)pNode;

        totalsize += pFile->m_FileLength;
    }

    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileObject* pFile = (MyFileObject*)pNode;

        totalsize += pFile->m_FileLength;
    }

    return totalsize;
}

MyFileObject* FileManager::CreateFileObject(const char* fullpath)
{
    MyFileObject* pFile = MyNew MyFileObject;

    pFile->ParseName( fullpath );

    m_FilesLoaded.AddTail( pFile );

    return pFile;
}

MyFileObject* FileManager::RequestFile(const char* filename)
{
    MyAssert( filename != 0 );
    if( filename == 0 )
        return 0;
    MyAssert( filename[0] != 0 );
    if( filename[0] == 0 )
        return 0;

    MyFileObject* pFile;

    // check if the file has already been requested... might still be loading.
    pFile = FindFileByName( filename );
    if( pFile )
    {
        pFile->AddRef();
        return pFile;
    }

    // if the file wasn't already loaded create a new one and load it up.
    pFile = 0;

    int len = (int)strlen( filename );
    if( len > 5 && strcmp( &filename[len-5], ".glsl" ) == 0 )
        pFile = MyNew MyFileObjectShader;
    else
        pFile = MyNew MyFileObject;

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
    for( CPPListNode* pNode = m_FilesLoaded.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        MyFileObject* pFile = (MyFileObject*)pNode;

        if( strcmp( filename, pFile->m_FullPath ) == 0 )
            return pFile;
    }

    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        MyFileObject* pFile = (MyFileObject*)pNode;

        if( strcmp( filename, pFile->m_FullPath ) == 0 )
            return pFile;
    }

    return 0;
}

// Move file into loaded list, call finished loading callbacks, add file to memory panel in editor
void FileManager::FinishSuccessfullyLoadingFile(MyFileObject* pFile)
{
    MyAssert( pFile->m_FileLoadStatus == FileLoadStatus_Success );

    m_FilesLoaded.MoveTail( pFile );

    // inform all registered objects that the file finished loading.
    for( CPPListNode* pNode = pFile->m_FileFinishedLoadingCallbackList.GetHead(); pNode != 0; )
    {
        CPPListNode* pNextNode = pNode->GetNext();

        FileFinishedLoadingCallbackStruct* pCallbackStruct = (FileFinishedLoadingCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, pFile );

        pNode = pNextNode;
    }

#if MYFW_USING_WX
    g_pPanelMemory->AddFile( pFile, pFile->m_ExtensionWithDot, pFile->m_FullPath, MyFileObject::StaticOnLeftClick, MyFileObject::StaticOnRightClick, MyFileObject::StaticOnDrag );
#endif
}

void FileManager::Tick()
{
#if USE_PTHREAD && !MYFW_NACL
    int threadindex = 0;

    // if there are no files to load, grab the mutex to lock the loading thread until there are.
    if( m_pFileThisFileIOThreadIsLoading[threadindex] == 0 )
    {
        if( m_FileIOThreadIsLocked[threadindex] == false ) // if we don't already have the lock, grab it.
        {
            pthread_mutex_lock( &m_FileIOThreadLocks[threadindex] );
            m_FileIOThreadIsLocked[threadindex] = true;
        }
    }

    // if we don't own the mutex for the fileio thread, then return
    if( m_FileIOThreadIsLocked[threadindex] == false )
        return;

    // The previously loading file is now loaded, mark it as successfully loaded, will "FinishSuccessfullyLoadingFile" below
    if( m_pLastFileLoadedByThread[threadindex] )
    {
        // if the file was successfully loaded by the other thread, then mark it loaded and call it's callbacks.
        if( m_pLastFileLoadedByThread[threadindex]->m_FileLoadStatus == FileLoadStatus_LoadedButNotFinalized )
        {
            m_pLastFileLoadedByThread[threadindex]->m_FileLoadStatus = FileLoadStatus_Success;
        }

        m_pLastFileLoadedByThread[threadindex] = 0;
    }
#endif //USE_PTHREAD && !MYFW_NACL

    // check if there are more files to load.
    {
#if USE_PTHREAD && !MYFW_NACL
        // the file being loaded should be 0 if we have the mutex.
        MyAssert( m_pFileThisFileIOThreadIsLoading[threadindex] == 0 );
#endif //USE_PTHREAD && !MYFW_NACL

        // continue to tick any files still in the "loading" queue.
        MyFileObject* pNextFile;
        for( MyFileObject* pFile = (MyFileObject*)m_FilesStillLoading.GetHead(); pFile != 0; pFile = pNextFile )
        {
			// Get the next file pointer
            pNextFile = (MyFileObject*)pFile->GetNext();

            //LOGInfo( LOGTag, "Loading File: %s\n", pFile->m_FullPath );

            // if the file already failed to load, give up on it.
            //   in editor mode: we reset m_LoadFailed when focus regained and try all files again.
            if( pFile->m_FileLoadStatus > FileLoadStatus_Success )
            {
                continue;
            }

            // Add a ref to this file to prevent it from being deleted while in this loop
            pFile->AddRef();

            // if we're done loading, move the file into the loaded list.
            if( pFile->m_FileLoadStatus == FileLoadStatus_Success )
            {
                //LOGInfo( LOGTag, "Finished loading: %s\n", pFile->m_FullPath );

                // Move file into loaded list, call finished loading callbacks, add file to memory panel in editor
                FinishSuccessfullyLoadingFile( pFile );
            }
            else
            {
#if USE_PTHREAD && !MYFW_NACL
                // release the mutex so the fileio thread can load the file we want.
                m_pLastFileLoadedByThread[threadindex] = pFile;
                m_pFileThisFileIOThreadIsLoading[threadindex] = pFile;
                pthread_mutex_unlock( &m_FileIOThreadLocks[threadindex] );
                m_FileIOThreadIsLocked[threadindex] = false;
#else
                {
#if !MYFW_NACL
                    pFile->Tick();
#if !USE_PTHREAD
                    pFile->m_FileLoadStatus = FileLoadStatus_Success;
#endif !USE_PTHREAD
#endif //!MYFW_NACL
                }
#endif // USE_PTHREAD && !MYFW_NACL

                pFile->Release(); // release the ref added above
                break; // file io thread only loads one file at a time.
            }

            // Release the ref added to this file up above
            pFile->Release();
        }
    }
}

int FileManager::ReloadAnyUpdatedFiles(FileManager_OnFileUpdated_CallbackFunction pCallbackFunc)
{
    int numfilesupdated = 0;

    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileObject* pFile = (MyFileObject*)pNode;

        bool updateavailable = pFile->IsNewVersionAvailable();

        if( updateavailable )
        {
            ReloadFile( pFile );
            pCallbackFunc( pFile );
            numfilesupdated++;
        }
    }

    for( CPPListNode* pNode = m_FilesLoaded.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileObject* pFile = (MyFileObject*)pNode;

        bool updateavailable = pFile->IsNewVersionAvailable();

        if( updateavailable )
        {
            ReloadFile( pFile );
            pCallbackFunc( pFile );
            numfilesupdated++;
        }
    }

    return numfilesupdated;
}

void FileManager::MoveFileToFrontOfFileLoadedList(MyFileObject* pFile)
{
    if( pFile == 0 )
        return;

    // make sure the file is in the loaded list.
    for( CPPListNode* pNode = m_FilesLoaded.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        MyFileObject* pFileLoaded = (MyFileObject*)pNode;
        if( pFileLoaded == pFile )
        {
            m_FilesLoaded.MoveHead( pFile );
            return;
        }
    }
}

#if MYFW_USING_WX
bool FileManager::DoesFileExist(const char* fullpath)
{
#if MYFW_WINDOWS
    WIN32_FIND_DATAA data;
    memset( &data, 0, sizeof( data ) );

    void GetFileData(const char* path, WIN32_FIND_DATAA* data); // in MyFileObject.cpp

    GetFileData( fullpath, &data );

    if( data.dwFileAttributes != 0 )
        return true;
#else
    return true;
#endif

    return false;
}

MyFileObject* FileManager::LoadFileNow(const char* fullpath)
{
    MyAssert( DoesFileExist( fullpath ) );

    MyFileObject* pFile = RequestFile( fullpath );

    while( pFile->m_FileLoadStatus < FileLoadStatus_Success )
    {
        pFile->Tick();

        if( pFile->m_FileLoadStatus == FileLoadStatus_LoadedButNotFinalized )
            pFile->m_FileLoadStatus = FileLoadStatus_Success;
    }

    if( pFile->m_FileLoadStatus == FileLoadStatus_Success )
    {
        // Move file into loaded list, call finished loading callbacks, add file to memory panel in editor
        FinishSuccessfullyLoadingFile( pFile );
    }

    return pFile;
}
#endif

MySaveFileObject* CreatePlatformSpecificSaveFile()
{
    MySaveFileObject* pSaveFile = 0;

#if MYFW_WINDOWS
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
    m_pFile = 0;
    m_pObjectToWriteBuffer = 0;
}

MySaveFileObject_FILE::~MySaveFileObject_FILE()
{
}

void MySaveFileObject_FILE::Reset()
{
    MySaveFileObject::Reset();

    if( m_pFile )
        fclose( m_pFile );
    m_pFile = 0;

    m_pObjectToWriteBuffer = 0;
}

void MySaveFileObject_FILE::WriteString(const char* path, const char* filename, const char* string)
{
    MyAssert( path != 0 );
    MyAssert( filename != 0 );
    MyAssert( string != 0 );
    MyAssert( m_pFile == 0 );

    if( m_pFile != 0 )
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
    MyAssert( path != 0 );
    MyAssert( filename != 0 );
    MyAssert( m_pFile == 0 );

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

    MyAssert( m_pFile != 0 );

    if( m_SaveFileOp == SFO_Read )
    {
        fseek( m_pFile, 0, SEEK_END );
        long length = ftell( m_pFile );
        if( length > 0 )
        {
            fseek( m_pFile, 0, SEEK_SET );
            m_pReadStringBuffer = MyNew char[length+1];

            fread( m_pReadStringBuffer, length, 1, m_pFile );
            m_pReadStringBuffer[length] = 0;
        }

        fclose( m_pFile );
        m_pFile = 0;
        m_OpComplete = true;
    }

    if( m_SaveFileOp == SFO_Write )
    {
        MyAssert( m_pObjectToWriteBuffer != 0 );

        int length = (int)strlen( m_pObjectToWriteBuffer );
        fwrite( m_pObjectToWriteBuffer, length, 1, m_pFile );
        fclose( m_pFile );
        m_pFile = 0;
        m_OpComplete = true;
    }
}

#if MYFW_WINDOWS || MYFW_BLACKBERRY || MYFW_EMSCRIPTEN || MYFW_IOS || MYFW_OSX
MyFileObject* RequestFile(const char* filename)
{
    MyAssert( filename != 0 );
    if( filename == 0 )
        return 0;
    MyAssert( filename[0] != 0 );
    if( filename[0] == 0 )
        return 0;

    return g_pFileManager->RequestFile( filename );
}
#endif

GLuint LoadTextureFromMemory(TextureDefinition* texturedef)
{
    //LOGInfo( LOGTag, "LoadTextureFromMemory texturedef(%d)", texturedef );
    //LOGInfo( LOGTag, "LoadTextureFromMemory texturedef->m_pFile(%d)", texturedef->m_pFile );
    //LOGInfo( LOGTag, "LoadTextureFromMemory texturedef->m_pFile->m_pBuffer(%d)", texturedef->m_pFile->m_pBuffer );

    unsigned char* buffer = (unsigned char*)texturedef->m_pFile->m_pBuffer;
    int length = texturedef->m_pFile->m_FileLength;

    unsigned char* pngbuffer;
    unsigned int width, height;

    unsigned int error = lodepng_decode32( &pngbuffer, &width, &height, buffer, length );
    MyAssert( error == 0 );

    GLuint texhandle = 0;
    glGenTextures( 1, &texhandle );
    MyAssert( texhandle != 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texhandle );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pngbuffer );
    checkGlError( "glTexImage2D" );
    free( pngbuffer );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texturedef->m_MinFilter ); //LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texturedef->m_MagFilter ); //GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texturedef->m_WrapS );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texturedef->m_WrapT );

    texturedef->m_Width = width;
    texturedef->m_Height = height;

    return texhandle;
}
