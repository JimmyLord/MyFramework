//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
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
#elif MYFW_IOS || MYFW_OSX
    const char* fullpath = filename;
    
    filehandle = IOS_fopen( fullpath );
#else
    const char* fullpath = filename;

    filehandle = fopen( fullpath, "rb" );
#endif

    char* filecontents = 0;

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long size = ftell( filehandle );
        rewind( filehandle );

        filecontents = MyNew char[size+1];

        // This allocation must land on 4-byte boundary.
        MyAssert( ((int)filecontents)%4 == 0 );

        fread( filecontents, size, 1, filehandle );
        filecontents[size] = 0;

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

    m_FullPath = 0;
    m_FilenameWithoutExtension = 0;
    m_ExtensionWithDot = 0;
    m_FileLoadStatus = FileLoadStatus_Loading;
    m_FileLength = 0;
    m_pBuffer = 0;
    m_BytesRead = 0;

#if MYFW_NACL
    m_pNaClFileObject = 0;
#endif

#if MYFW_WINDOWS
    m_FileLastWriteTime.dwHighDateTime = 0;
    m_FileLastWriteTime.dwLowDateTime = 0;
#endif

#if MYFW_USING_WX
    m_CustomLeftClickCallback = 0;
    m_CustomLeftClickObject = 0;
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

    SAFE_DELETE_ARRAY( m_FullPath );
    SAFE_DELETE_ARRAY( m_FilenameWithoutExtension );
    SAFE_DELETE_ARRAY( m_ExtensionWithDot );
    SAFE_DELETE_ARRAY( m_pBuffer );

#if MYFW_NACL
    SAFE_DELETE( m_pNaClFileObject );
#endif

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveFile( this );
#endif
}

void MyFileObject::GenerateNewFullPathFilenameInSameFolder(char* newfilename, char* buffer, int buffersize)
{
    MyAssert( buffer != 0 );
    sprintf_s( buffer, buffersize, "%s", m_FullPath );
    int endoffolderoffset = (int)( strlen(m_FullPath) - strlen(m_FilenameWithoutExtension) - strlen(m_ExtensionWithDot) );
    sprintf_s( &buffer[endoffolderoffset], buffersize - endoffolderoffset, "%s", newfilename );
}

void MyFileObject::GenerateNewFullPathExtensionWithSameNameInSameFolder(const char* newextension, char* buffer, int buffersize)
{
    MyAssert( buffer != 0 );
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
            strncpy_s( g_FolderName, namelen+1, &m_FullPath[i], namelen );
            g_FolderName[namelen] = 0;
            return g_FolderName;
        }
        i--;
    }

    return "";
}

const char* MyFileObject::Rename(const char* newnamewithoutextension)
{
#if MYFW_USING_WX
    char fullpathbefore[MAX_PATH];
    sprintf_s( fullpathbefore, MAX_PATH, "%s", m_FullPath );
#endif

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

#if MYFW_USING_WX
        g_pGameCore->OnFileRenamed( fullpathbefore, m_FullPath );
#endif
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

void MyFileObject::RegisterFileFinishedLoadingCallback(void* pObj, FileFinishedLoadingCallbackFunc pCallback)
{
    MyAssert( pCallback != 0 );

    FileFinishedLoadingCallbackStruct* pCallbackStruct = g_pMyFileObject_FileFinishedLoadingCallbackPool.GetObjectFromPool();

    if( pCallbackStruct != 0 )
    {
        pCallbackStruct->pObj = pObj;
        pCallbackStruct->pFunc = pCallback;

        m_FileFinishedLoadingCallbackList.AddTail( pCallbackStruct );
    }
}

void MyFileObject::UnregisterFileFinishedLoadingCallback(void* pObj)
{
    for( CPPListNode* pNode = m_FileFinishedLoadingCallbackList.GetHead(); pNode != 0; )
    {
        CPPListNode* pNextNode = pNode->GetNext();

        FileFinishedLoadingCallbackStruct* pCallbackStruct = (FileFinishedLoadingCallbackStruct*)pNode;

        if( pCallbackStruct->pObj == pObj )
        {
            pCallbackStruct->Remove();
            g_pMyFileObject_FileFinishedLoadingCallbackPool.ReturnObjectToPool( pCallbackStruct );
        }

        pNode = pNextNode;
    }
}


void MyFileObject::RequestFile(const char* filename)
{
    MyAssert( filename != 0 );
    if( filename == 0 )
        return;
    MyAssert( filename[0] != 0 );
    if( filename[0] == 0 )
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
                m_ExtensionWithDot[extensionlen] = 0;
                break;
            }
            extensionstartlocation--;
        }

        if( m_ExtensionWithDot == 0 )
        {
            m_ExtensionWithDot = MyNew char[2];
            m_ExtensionWithDot[0] = '.';
            m_ExtensionWithDot[1] = 0;

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
                m_FilenameWithoutExtension[namelen] = 0;
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

        if( buffer == 0 )
        {
            LOGError( LOGTag, "FileLoadStatus_Error_FileNotFound %s\n", m_FullPath );
            m_FileLoadStatus = FileLoadStatus_Error_FileNotFound; // file not found.
            return;
        }

        if( length > 0 && buffer != 0 )
        {
            FakeFileLoad( buffer, length );
        }

        UpdateTimestamp();
    }
}

bool MyFileObject::IsNewVersionAvailable()
{
    bool updated = false;

    // If the file load status was an error.
    if( m_FileLoadStatus > FileLoadStatus_Success )
        m_FileLoadStatus = FileLoadStatus_Loading; // file now exists? allow it to load.

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
#endif

    return updated;
}

void MyFileObject::UpdateTimestamp()
{
#if MYFW_WINDOWS
    WIN32_FIND_DATAA data;
    GetFileData( m_FullPath, &data );

    m_FileLastWriteTime = data.ftLastWriteTime;
#endif
}

void MyFileObject::UnloadContents()
{
    SAFE_DELETE_ARRAY( m_pBuffer );
    m_FileLength = 0;
    m_BytesRead = 0;
    m_FileLoadStatus = FileLoadStatus_Loading;

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveFile( this );
#endif
}

void MyFileObject::FakeFileLoad(char* buffer, int length)
{
    MyAssert( buffer != 0 && length > 0 );
    if( buffer == 0 || length <= 0 )
        return;

    m_pBuffer = buffer;
    m_FileLength = length;
    m_BytesRead = length;
    m_FileLoadStatus = FileLoadStatus_LoadedButNotFinalized;
}

#if MYFW_USING_WX
void MyFileObject::OnLeftClick(unsigned int count)
{
}

void MyFileObject::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );
    
    menu.Append( RightClick_ViewInWatchWindow, "View in watch window" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&MyFileObject::OnPopupClick );

    menu.Append( RightClick_OpenFile, "Open file" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&MyFileObject::OnPopupClick );

    menu.Append( RightClick_OpenContainingFolder, "Open containing folder" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&MyFileObject::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

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

    ShellExecuteA( 0, 0, url, 0, 0, SW_SHOWNORMAL );
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
    params[endoffolderoffset+1] = 0;
    for( int i=0; i<endoffolderoffset; i++ )
    {
        if( params[i] == '/' )
        {
            params[i] = '\\';
        }
    }

    ShellExecuteA( 0, 0, "explorer.exe", params, 0, SW_SHOWNORMAL );
#endif
}

void MyFileObject::OnPopupClick(wxEvent &evt)
{
    MyFileObject* pFileObject = (MyFileObject*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    int id = evt.GetId();
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
            g_pPanelWatch->ClearAllVariables();

            g_pPanelWatch->AddSpace( pFileObject->m_FullPath );

            if( strcmp( pFileObject->m_ExtensionWithDot, ".mymesh" ) == 0 )
            {
                MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFileObject );

                pMesh->FillPropertiesWindow( false );
            }

            //// TODO: not sure what this was for and yet still caused occasional crashes, removed for now.
            ////       also, left click callback on right click event?!?
            //if( m_CustomLeftClickCallback )
            //{
            //    m_CustomLeftClickCallback( m_CustomLeftClickObject );
            //}
        }
        break;
    }
}

void MyFileObject::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_FileObjectPointer;
    g_DragAndDropStruct.m_Value = this;
}

void MyFileObject::SetCustomLeftClickCallback(PanelObjectListObjectCallback callback, void* object)
{
    m_CustomLeftClickObject = object;
    m_CustomLeftClickCallback = callback;
}
#endif //MYFW_USING_WX
