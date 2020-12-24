//
// Copyright (c) 2012-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#if MYFW_WINDOWS
#include <direct.h>
#endif

#if MYFW_ANDROID
#include <sys/stat.h>
#include "../../SourceAndroid/AndroidFileLoader.h"
#endif

#include "FileManager.h"
#include "MyFileObject.h"

#if MYFW_WINDOWS
void GetFileData(const char* path, WIN32_FIND_DATAA* data)
{
    HANDLE handle = FindFirstFileA( path, data );

    if( handle != INVALID_HANDLE_VALUE )
        FindClose( handle );
}

FileTimeStamp GetFileLastModifiedTime(const char* path)
{
    WIN32_FIND_DATAA data;
    memset( &data, 0, sizeof( data ) );
    GetFileData( path, &data );

    FileTimeStamp timeStamp = (uint64)(data.ftLastWriteTime.dwHighDateTime) << 32 | (uint64)data.ftLastWriteTime.dwLowDateTime;
    return timeStamp;
}
#else
FileTimeStamp GetFileLastModifiedTime(const char* path)
{
    MyAssert( false ); // Test this.

    struct stat data;
    stat( path, &data );
    return data.st_mtime;
}
#endif

char* PlatformSpecific_LoadFile(const char* relativePath, int* length, const char* file, unsigned long line)
{
    FILE* fileHandle;

#if MYFW_ANDROID
    return LoadFile( relativePath, length );
#endif

#if MYFW_WINDOWS
    const char* fullPath = relativePath;

    errno_t error = fopen_s( &fileHandle, fullPath, "rb" );
#elif MYFW_BLACKBERRY
    char fullpath[MAX_PATH];
    sprintf_s( fullpath, MAX_PATH, "app/native/%s", relativePath );

    fileHandle = fopen( fullpath, "rb" );
#elif MYFW_OSX
    char currentdir[PATH_MAX];
    getcwd( currentdir, PATH_MAX );

    const char* fullPath = relativePath;

    fileHandle = fopen( relativePath, "rb" );
#elif MYFW_IOS
    const char* fullPath = relativePath;
    
    fileHandle = IOS_fopen( fullpath );
#else
    const char* fullPath = relativePath;

    fileHandle = fopen( fullPath, "rb" );
#endif

    char* fileContents = nullptr;

    if( fileHandle )
    {
        fseek( fileHandle, 0, SEEK_END );
        long size = ftell( fileHandle );
        rewind( fileHandle );

        fileContents = MyNew char[size+1];

        // This allocation must land on 4-byte boundary.
        MyAssert( ((uintptr_t)fileContents)%4 == 0 );

        fread( fileContents, size, 1, fileHandle );
        fileContents[size] = '\0';

        if( length )
            *length = (int)size+1;
        
        fclose( fileHandle );
    }
    else
    {
        LOGError( LOGTag, "File not found: %s\n", fullPath );
    }

    return fileContents;
}

MyFileObject::MyFileObject(FileManager* pFileManager)
{
    m_pFileManager = pFileManager;

    ClassnameSanityCheck();

    m_FullPath = nullptr;
    m_FilenameWithoutExtension = nullptr;
    m_ExtensionWithDot = nullptr;
    m_FileLoadStatus = FileLoadStatus_Loading;
    m_FileLength = 0;
    m_pBuffer = nullptr;
    m_BytesRead = 0;

#if MYFW_NACL
    m_pNaClFileObject = nullptr;
#endif

#if MYFW_WINDOWS
    m_FileLastWriteTime.dwHighDateTime = 0;
    m_FileLastWriteTime.dwLowDateTime = 0;
#else
    m_FileLastWriteTime = 0;
#endif

#if MYFW_EDITOR
    m_ShowInMemoryPanel = true;
#endif
}

MyFileObject::~MyFileObject()
{
    // Make sure you call ->Release.  don't delete a file object, it's refcounted.
#if MYFW_WINDOWS
    MyAssert( Prev );
#endif
    if( Prev ) // If it's in a list... it isn't on some? platforms ATM, need to update file loaders on each.
        Remove();

    MyAssert( m_FileFinishedLoadingCallbackList.GetHead() == nullptr );

    SAFE_DELETE_ARRAY( m_FullPath );
    SAFE_DELETE_ARRAY( m_FilenameWithoutExtension );
    SAFE_DELETE_ARRAY( m_ExtensionWithDot );
    SAFE_DELETE_ARRAY( m_pBuffer );

#if MYFW_NACL
    SAFE_DELETE( m_pNaClFileObject );
#endif
}

void MyFileObject::SystemStartup()
{
}

void MyFileObject::SystemShutdown()
{
}

void MyFileObject::GenerateNewFullPathFilenameInSameFolder(char* newFilename, char* buffer, int bufferSize)
{
    MyAssert( buffer != nullptr );
    sprintf_s( buffer, bufferSize, "%s", m_FullPath );
    int endOfFolderOffset = (int)( strlen(m_FullPath) - strlen(m_FilenameWithoutExtension) - strlen(m_ExtensionWithDot) );
    sprintf_s( &buffer[endOfFolderOffset], bufferSize - endOfFolderOffset, "%s", newFilename );
}

void MyFileObject::GenerateNewFullPathExtensionWithSameNameInSameFolder(const char* newExtension, char* buffer, int bufferSize)
{
    MyAssert( buffer != nullptr );
    sprintf_s( buffer, bufferSize, "%s", m_FullPath );
    int endOfFilenameOffset = (int)( strlen(m_FullPath) - strlen(m_ExtensionWithDot) );
    sprintf_s( &buffer[endOfFilenameOffset], bufferSize - endOfFilenameOffset, "%s", newExtension );
}

static char g_FolderName[MAX_PATH];

const char* MyFileObject::GetNameOfDeepestFolderPath()
{
    int len = (int)strlen( m_FullPath );
    MyAssert( len > 0 );
    if( len <= 0 )
        return "";

    int folderStartLocation = len;
    int i = folderStartLocation;
    while( i >= 0 )
    {
        if( m_FullPath[i] == '/' )
        {
            folderStartLocation = i;
            i--;
            break;
        }
        i--;
    }

    while( i >= 0 )
    {
        if( i == 0 || m_FullPath[i] == '/' )
        {
            if( m_FullPath[i] == '/' )
                i++;
            int nameLen = folderStartLocation-i;
            strncpy_s( g_FolderName, MAX_PATH, &m_FullPath[i], nameLen );
            g_FolderName[nameLen] = '\0';
            return g_FolderName;
        }
        i--;
    }

    return "";
}

const char* MyFileObject::Rename(const char* newNameWithoutExtension)
{
    char newFullPath[MAX_PATH];

    int fullPathLen = (int)strlen( m_FullPath );
    int nameExtLen = (int)strlen( m_FilenameWithoutExtension ) + (int)strlen( m_ExtensionWithDot );
    int pathLen = fullPathLen - nameExtLen;

    sprintf_s( newFullPath, MAX_PATH, "%s", m_FullPath );
    sprintf_s( &newFullPath[pathLen], MAX_PATH-pathLen, "%s%s", newNameWithoutExtension, m_ExtensionWithDot );

    int result = rename( m_FullPath, newFullPath );
    if( result == 0 )
    {
        // Successfully renamed.
        ParseName( newFullPath );
    }
    else
    {
        // Failed to rename.
        LOGError( LOGTag, "Failed to rename %s to %s", m_FullPath, newNameWithoutExtension );
    }

    return m_FilenameWithoutExtension;
}

bool MyFileObject::IsFinishedLoading()
{
    if( m_FileLoadStatus < FileLoadStatus_Success ) // Still loading.
        return false;

    return true;
}

const char* MyFileObject::GetFilename()
{
    if( m_FullPath && m_FullPath[0] != '\0' )
    {
        int i;
        for( i=(int)strlen(m_FullPath)-1; i>=0; i-- )
        {
            if( m_FullPath[i] == '\\' || m_FullPath[i] == '/' )
                break;
        }
        return &m_FullPath[i+1];
    }

    return nullptr;
}

void MyFileObject::RegisterFileFinishedLoadingCallback(void* pObj, FileFinishedLoadingCallbackFunc* pCallback)
{
    MyAssert( pCallback != nullptr );

    FileFinishedLoadingCallbackStruct* pCallbackStruct = m_pFileManager->GetFileFinishedLoadingCallbackPool()->GetObjectFromPool();

#if _DEBUG
    // Assert this callback isn't registered multiple times.
    for( FileFinishedLoadingCallbackStruct* pCallbackStruct = m_FileFinishedLoadingCallbackList.GetHead(); pCallbackStruct != nullptr; pCallbackStruct = pCallbackStruct->GetNext() )
    {
        MyAssert( pCallbackStruct->pObj != pObj || pCallbackStruct->pFunc != pCallback );
    }
#endif

    if( pCallbackStruct != nullptr )
    {
        pCallbackStruct->pObj = pObj;
        pCallbackStruct->pFunc = pCallback;

        m_FileFinishedLoadingCallbackList.AddTail( pCallbackStruct );
    }
}

void MyFileObject::UnregisterFileFinishedLoadingCallback(void* pObj)
{
    for( FileFinishedLoadingCallbackStruct* pCallbackStruct = m_FileFinishedLoadingCallbackList.GetHead(); pCallbackStruct != nullptr; )
    {
        FileFinishedLoadingCallbackStruct* pNextCallbackStruct = pCallbackStruct->GetNext();

        if( pCallbackStruct->pObj == pObj )
        {
            pCallbackStruct->Remove();
            m_pFileManager->GetFileFinishedLoadingCallbackPool()->ReturnObjectToPool( pCallbackStruct );
        }

        pCallbackStruct = pNextCallbackStruct;
    }
}


void MyFileObject::RequestFile(const char* filename)
{
    MyAssert( filename != nullptr );
    if( filename == nullptr )
        return;
    MyAssert( filename[0] != '\0' );
    if( filename[0] == '\0' )
        return;

    ParseName( filename );
}

void MyFileObject::ParseName(const char* filename)
{
    SAFE_DELETE_ARRAY( m_FullPath );
    SAFE_DELETE_ARRAY( m_FilenameWithoutExtension );
    SAFE_DELETE_ARRAY( m_ExtensionWithDot );

    int len = (int)strlen( filename );
    MyAssert( len > 0 );
    if( len <= 0 )
        return;

    m_FullPath = MyNew char[len+1];
    strcpy_s( m_FullPath, len+1, filename );

    int extensionStartLocation = len;
    {
        while( extensionStartLocation > 0 )
        {
            if( filename[extensionStartLocation] == '.' )
            {
                int extensionLen = len-extensionStartLocation;
                m_ExtensionWithDot = MyNew char[extensionLen+1];
                strncpy_s( m_ExtensionWithDot, extensionLen+1, &filename[extensionStartLocation], extensionLen );
                m_ExtensionWithDot[extensionLen] = '\0';
                break;
            }
            extensionStartLocation--;
        }

        if( m_ExtensionWithDot == nullptr )
        {
            m_ExtensionWithDot = MyNew char[2];
            m_ExtensionWithDot[0] = '.';
            m_ExtensionWithDot[1] = '\0';

            extensionStartLocation = len;
        }
    }

    {
        int i = extensionStartLocation;
        while( i >= 0 )
        {
            if( i == 0 || filename[i] == '/' || filename[i] == '\\' )
            {
                if( filename[i] == '/' || filename[i] == '\\' )
                    i++;
                int nameLen = extensionStartLocation-i;
                m_FilenameWithoutExtension = MyNew char[nameLen+1];
                strncpy_s( m_FilenameWithoutExtension, nameLen+1, &filename[i], nameLen );
                m_FilenameWithoutExtension[nameLen] = '\0';
                break;
            }
            i--;
        }
    }
}

void MyFileObject::Tick()
{
    if( m_FileLoadStatus != FileLoadStatus_Success )
    {
        int length = 0;

        const char* fullPath = m_FullPath;

#if MYFW_WINDOWS
        // Despite the first arg of PlatformSpecific_LoadFile being "relativePath",
        //     on Windows, make sure relativePath is actually a complete path.
        // Relative paths will only cause issues in cases where files are being loaded
        //     from multiple working directories on different threads.
        // See similar issue in MyFileObject::UpdateTimestamp()
        char tempPath[MAX_PATH];
        sprintf_s( tempPath, MAX_PATH, "%s/%s", m_pFileManager->GetWorkingDirectory(), m_FullPath );
        fullPath = tempPath;
#endif

        char* buffer = PlatformSpecific_LoadFile( fullPath, &length, m_FullPath, __LINE__ );

        if( buffer == nullptr )
        {
            LOGError( LOGTag, "FileLoadStatus_Error_FileNotFound %s\n", m_FullPath );
            m_FileLoadStatus = FileLoadStatus_Error_FileNotFound; // file not found.
            return;
        }

        if( length > 0 && buffer != nullptr )
        {
            FakeFileLoad( buffer, length );
        }

        UpdateTimestamp();
    }
}

bool MyFileObject::IsNewVersionAvailable()
{
    if( m_FileLoadStatus == FileLoadStatus_Loading )
        return false;

    bool updated = false;

    // If the file load status was an error.
    if( m_FileLoadStatus > FileLoadStatus_Success )
        m_FileLoadStatus = FileLoadStatus_Loading; // File now exists? allow it to load.

    // Read and store the new files timestamp if it changed.
#if MYFW_WINDOWS
    WIN32_FIND_DATAA data;
    memset( &data, 0, sizeof( data ) );
    GetFileData( m_FullPath, &data );

    if( data.ftLastWriteTime.dwHighDateTime != m_FileLastWriteTime.dwHighDateTime ||
        data.ftLastWriteTime.dwLowDateTime != m_FileLastWriteTime.dwLowDateTime )
    {
        updated = true;
        
        m_FileLastWriteTime = data.ftLastWriteTime;
    }
#else
    struct stat data;
    stat( m_FullPath, &data );
    if( m_FileLastWriteTime != data.st_mtime )
    {
        updated = true;

        m_FileLastWriteTime = data.st_mtime;
    }
#endif

    return updated;
}

void MyFileObject::UpdateTimestamp()
{
#if MYFW_WINDOWS
    // m_FullPath is a path relative to the working dir, we need this to check a complete Windows path.
    // Relative paths will only cause issues in cases where files are being loaded
    //     from multiple working directories on different threads.
    // See similar issue in MyFileObject::Tick()
    MyAssert( m_FullPath && m_FullPath[1] != ':' );
    char completeWindowsPath[MAX_PATH];
    sprintf_s( completeWindowsPath, MAX_PATH, "%s/%s", m_pFileManager->GetWorkingDirectory(), m_FullPath );

    WIN32_FIND_DATAA data;
    GetFileData( completeWindowsPath, &data );

    m_FileLastWriteTime = data.ftLastWriteTime;
#else
    struct stat data;
    stat( m_FullPath, &data );
    m_FileLastWriteTime = data.st_mtime;
#endif
}

void MyFileObject::UnloadContents()
{
    SAFE_DELETE_ARRAY( m_pBuffer );
    m_FileLength = 0;
    m_BytesRead = 0;
    m_FileLoadStatus = FileLoadStatus_Loading;
}

void MyFileObject::FakeFileLoad(char* buffer, int length)
{
    MyAssert( buffer != nullptr && length > 0 );
    if( buffer == nullptr || length <= 0 )
        return;

    m_pBuffer = buffer;
    m_FileLength = length;
    m_BytesRead = length;
    m_FileLoadStatus = FileLoadStatus_LoadedButNotFinalized;
}

#if MYFW_EDITOR
void MyFileObject::OSLaunchFile(bool createFileIfDoesntExist)
{
#if MYFW_WINDOWS
    char url[MAX_PATH];
    char workingdir[MAX_PATH];
    _getcwd( workingdir, MAX_PATH * sizeof(char) );
    sprintf_s( url, MAX_PATH, "%s/%s", workingdir, m_FullPath );

    if( FileManager::DoesFileExist( url ) == false )
    {
        FILE* file;
        fopen_s( &file, url, "wb" );

        if( file )
        {
            fclose( file );
        }
    }

    ShellExecuteA( 0, nullptr, url, nullptr, nullptr, SW_SHOWNORMAL );
#endif
}

void MyFileObject::OSOpenContainingFolder()
{
#if MYFW_WINDOWS
    char params[MAX_PATH];
    char workingDir[MAX_PATH];
    _getcwd( workingDir, MAX_PATH * sizeof(char) );
    sprintf_s( params, MAX_PATH, "\"%s/%s\"", workingDir, m_FullPath );

    int endOfFolderOffset = (int)( strlen(params) - strlen(m_FilenameWithoutExtension) - strlen(m_ExtensionWithDot) - 1);
    params[endOfFolderOffset] = '"';
    params[endOfFolderOffset+1] = '\0';
    for( int i=0; i<endOfFolderOffset; i++ )
    {
        if( params[i] == '/' )
        {
            params[i] = '\\';
        }
    }

    ShellExecuteA( 0, nullptr, "explorer.exe", params, nullptr, SW_SHOWNORMAL );
#endif
}

void MyFileObject::OnPopupClick(FileManager* pFileManager, MyFileObject* pFileObject, int id)
{
    switch( id )
    {
    case RightClick_OpenFile:
        {
            pFileObject->OSLaunchFile( true );
        }
        break;

    case RightClick_OpenContainingFolder:
        {
            pFileObject->OSOpenContainingFolder();
        }
        break;

    case RightClick_ViewInWatchWindow:
        {
        }
        break;

    case RightClick_UnloadFile:
        {
            pFileManager->Editor_UnloadFile( pFileObject );
        }
        break;

    case RightClick_FindAllReferences:
        {
            pFileManager->Editor_FindAllReferences( pFileObject );
        }
        break;
    }
}
#endif //MYFW_EDITOR
