//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

MySimplePool<FileFinishedLoadingCallbackStruct> g_pMyFileObject_FileFinishedLoadingCallbackPool;

#if MYFW_WINDOWS
void GetFileData(const char* path, WIN32_FIND_DATAA* data)
{
    HANDLE handle = FindFirstFileA( path, data );

    if( handle != INVALID_HANDLE_VALUE )
        FindClose( handle );
}
#endif

char* PlatformSpecific_LoadFile(const char* filename, int* length, const char* file, unsigned long line)
{
    FILE* filehandle;

#if MYFW_ANDROID
    return LoadFile( filename, length );
#endif

#if MYFW_WINDOWS
    const char* fullpath = filename;

    errno_t error = fopen_s( &filehandle, fullpath, "rb" );
#elif MYFW_BLACKBERRY
    char fullpath[MAX_PATH];
    sprintf_s( fullpath, MAX_PATH, "app/native/%s", filename );

    filehandle = fopen( fullpath, "rb" );
#elif MYFW_OSX
    char currentdir[PATH_MAX];
    getcwd( currentdir, PATH_MAX );

    const char* fullpath = filename;

    filehandle = fopen( filename, "rb" );
#elif MYFW_IOS
    const char* fullpath = filename;
    
    filehandle = IOS_fopen( fullpath );
#else
    const char* fullpath = filename;

    filehandle = fopen( fullpath, "rb" );
#endif

    char* filecontents = nullptr;

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long size = ftell( filehandle );
        rewind( filehandle );

        filecontents = MyNew char[size+1];

        // This allocation must land on 4-byte boundary.
        MyAssert( ((uintptr_t)filecontents)%4 == 0 );

        fread( filecontents, size, 1, filehandle );
        filecontents[size] = '\0';

        if( length )
            *length = (int)size+1;
        
        fclose( filehandle );
    }
    else
    {
        LOGError( LOGTag, "File not found: %s\n", fullpath );
    }

    return filecontents;
}

MyFileObject::MyFileObject()
{
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

    // the first MyFileObject will create the pool of callback objects.
    if( g_pMyFileObject_FileFinishedLoadingCallbackPool.IsInitialized() == false )
        g_pMyFileObject_FileFinishedLoadingCallbackPool.AllocateObjects( CALLBACK_POOL_SIZE );
}

MyFileObject::~MyFileObject()
{
    // make sure you call ->Release.  don't delete a file object, it's refcounted.
#if MYFW_WINDOWS
    MyAssert( Prev );
#endif
    if( Prev ) // if it's in a list... it isn't on some? platforms ATM, need to update file loaders on each.
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

void MyFileObject::GenerateNewFullPathFilenameInSameFolder(char* newfilename, char* buffer, int buffersize)
{
    MyAssert( buffer != nullptr );
    sprintf_s( buffer, buffersize, "%s", m_FullPath );
    int endoffolderoffset = (int)( strlen(m_FullPath) - strlen(m_FilenameWithoutExtension) - strlen(m_ExtensionWithDot) );
    sprintf_s( &buffer[endoffolderoffset], buffersize - endoffolderoffset, "%s", newfilename );
}

void MyFileObject::GenerateNewFullPathExtensionWithSameNameInSameFolder(const char* newextension, char* buffer, int buffersize)
{
    MyAssert( buffer != nullptr );
    sprintf_s( buffer, buffersize, "%s", m_FullPath );
    int endoffilenameoffset = (int)( strlen(m_FullPath) - strlen(m_ExtensionWithDot) );
    sprintf_s( &buffer[endoffilenameoffset], buffersize - endoffilenameoffset, "%s", newextension );
}

static char g_FolderName[MAX_PATH];

const char* MyFileObject::GetNameOfDeepestFolderPath()
{
    int len = (int)strlen( m_FullPath );
    MyAssert( len > 0 );
    if( len <= 0 )
        return "";

    int folderstartlocation = len;
    int i = folderstartlocation;
    while( i >= 0 )
    {
        if( m_FullPath[i] == '/' )
        {
            folderstartlocation = i;
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
            int namelen = folderstartlocation-i;
            strncpy_s( g_FolderName, MAX_PATH, &m_FullPath[i], namelen );
            g_FolderName[namelen] = '\0';
            return g_FolderName;
        }
        i--;
    }

    return "";
}

const char* MyFileObject::Rename(const char* newnamewithoutextension)
{
    char newfullpath[MAX_PATH];

    int fullpathlen = (int)strlen( m_FullPath );
    int nameextlen = (int)strlen( m_FilenameWithoutExtension ) + (int)strlen( m_ExtensionWithDot );
    int pathlen = fullpathlen - nameextlen;

    sprintf_s( newfullpath, MAX_PATH, "%s", m_FullPath );
    sprintf_s( &newfullpath[pathlen], MAX_PATH-pathlen, "%s%s", newnamewithoutextension, m_ExtensionWithDot );

    int result = rename( m_FullPath, newfullpath );
    if( result == 0 )
    {
        // successfully renamed
        ParseName( newfullpath );
    }
    else
    {
        // failed to rename
        LOGError( LOGTag, "Failed to rename %s to %s", m_FullPath, newnamewithoutextension );
    }

    return m_FilenameWithoutExtension;
}

bool MyFileObject::IsFinishedLoading()
{
    if( m_FileLoadStatus < FileLoadStatus_Success ) // still loading
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

    return 0;
}

void MyFileObject::RegisterFileFinishedLoadingCallback(void* pObj, FileFinishedLoadingCallbackFunc pCallback)
{
    MyAssert( pCallback != nullptr );

    FileFinishedLoadingCallbackStruct* pCallbackStruct = g_pMyFileObject_FileFinishedLoadingCallbackPool.GetObjectFromPool();

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
            g_pMyFileObject_FileFinishedLoadingCallbackPool.ReturnObjectToPool( pCallbackStruct );
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

    int extensionstartlocation = len;
    {
        while( extensionstartlocation > 0 )
        {
            if( filename[extensionstartlocation] == '.' )
            {
                int extensionlen = len-extensionstartlocation;
                m_ExtensionWithDot = MyNew char[extensionlen+1];
                strncpy_s( m_ExtensionWithDot, extensionlen+1, &filename[extensionstartlocation], extensionlen );
                m_ExtensionWithDot[extensionlen] = '\0';
                break;
            }
            extensionstartlocation--;
        }

        if( m_ExtensionWithDot == nullptr )
        {
            m_ExtensionWithDot = MyNew char[2];
            m_ExtensionWithDot[0] = '.';
            m_ExtensionWithDot[1] = '\0';

            extensionstartlocation = len;
        }
    }

    {
        int i = extensionstartlocation;
        while( i >= 0 )
        {
            if( i == 0 || filename[i] == '/' || filename[i] == '\\' )
            {
                if( filename[i] == '/' || filename[i] == '\\' )
                    i++;
                int namelen = extensionstartlocation-i;
                m_FilenameWithoutExtension = MyNew char[namelen+1];
                strncpy_s( m_FilenameWithoutExtension, namelen+1, &filename[i], namelen );
                m_FilenameWithoutExtension[namelen] = '\0';
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

        char* buffer = PlatformSpecific_LoadFile( m_FullPath, &length, m_FullPath, __LINE__ );

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
        m_FileLoadStatus = FileLoadStatus_Loading; // file now exists? allow it to load.

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
    WIN32_FIND_DATAA data;
    GetFileData( m_FullPath, &data );

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
void MyFileObject::OSLaunchFile(bool createfileifdoesntexist)
{
#if MYFW_WINDOWS
    char url[MAX_PATH];
    char workingdir[MAX_PATH];
    _getcwd( workingdir, MAX_PATH * sizeof(char) );
    sprintf_s( url, MAX_PATH, "%s/%s", workingdir, m_FullPath );

    if( g_pFileManager->DoesFileExist( url ) == false )
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
    char workingdir[MAX_PATH];
    _getcwd( workingdir, MAX_PATH * sizeof(char) );
    sprintf_s( params, MAX_PATH, "\"%s/%s\"", workingdir, m_FullPath );

    int endoffolderoffset = (int)( strlen(params) - strlen(m_FilenameWithoutExtension) - strlen(m_ExtensionWithDot) - 1);
    params[endoffolderoffset] = '"';
    params[endoffolderoffset+1] = '\0';
    for( int i=0; i<endoffolderoffset; i++ )
    {
        if( params[i] == '/' )
        {
            params[i] = '\\';
        }
    }

    ShellExecuteA( 0, nullptr, "explorer.exe", params, nullptr, SW_SHOWNORMAL );
#endif
}

void MyFileObject::OnPopupClick(MyFileObject* pFileObject, int id)
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
            g_pFileManager->Editor_UnloadFile( pFileObject );
        }
        break;

    case RightClick_FindAllReferences:
        {
            g_pFileManager->Editor_FindAllReferences( pFileObject );
        }
        break;
    }
}
#endif //MYFW_EDITOR
