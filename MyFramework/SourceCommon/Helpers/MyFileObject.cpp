//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

char* PlatformSpecific_LoadFile(const char* filename, int* length = 0, const char* file = __FILE__, unsigned long line = __LINE__);

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

#if MYFW_WINDOWS
    const char* fullpath = filename;

    errno_t error = fopen_s( &filehandle, fullpath, "rb" );
#elif MYFW_BLACKBERRY
    char fullpath[MAX_PATH];
    sprintf_s( fullpath, MAX_PATH, "app/native/%s", filename );

    filehandle = fopen( fullpath, "rb" );
#elif MYFW_IOS
    const char* fullpath = filename;
    
    filehandle = IOS_fopen( fullpath );
#else
    char* fullpath = filename;

    filehandle = fopen( fullpath, "rb" );
#endif

    char* filecontents = 0;

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long size = ftell( filehandle );
        rewind( filehandle );

        filecontents = MyNew char[size+1];
        //filecontents = new(__FILE__, __LINE__) char[size];
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
    m_FullPath = 0;
    m_FilenameWithoutExtension = 0;
    m_ExtensionWithDot = 0;
    m_FileLoadStatus = FileLoadStatus_Loading;
    m_FileLength = 0;
    m_pBuffer = 0;
    m_BytesRead = 0;

#if MYFW_WINDOWS
    m_FileLastWriteTime.dwHighDateTime = 0;
    m_FileLastWriteTime.dwLowDateTime = 0;
#endif

#if _DEBUG
    m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading = 2;
#else
    m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading = 0;
#endif

#if MYFW_USING_WX
    m_CustomLeftClickCallback = 0;
#endif
}

MyFileObject::~MyFileObject()
{
    // make sure you call ->Release.  don't delete a file object, it's refcounted.
#if MYFW_WINDOWS
    assert( Prev );
#endif
    if( Prev ) // if it's in a list... it isn't on some? platforms ATM, need to update file loaders on each.
        Remove();

    SAFE_DELETE_ARRAY( m_FullPath );
    SAFE_DELETE_ARRAY( m_FilenameWithoutExtension );
    SAFE_DELETE_ARRAY( m_ExtensionWithDot );
    SAFE_DELETE_ARRAY( m_pBuffer );

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveFile( this );
#endif
}

#if MYFW_USING_WX
void MyFileObject::OnLeftClick()
{
    g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->AddSpace( this->m_FullPath );

    if( strcmp( m_ExtensionWithDot, ".mymesh" ) == 0 )
    {
        MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( this );

        pMesh->FillPropertiesWindow( false );
    }

    //g_pPanelWatch->AddVector3( "Pos", &m_Position, -1.0f, 1.0f, this, ComponentTransform::StaticOnValueChanged );
    //g_pPanelWatch->AddVector3( "Scale", &m_Scale, 0.0f, 10.0f, this, ComponentTransform::StaticOnValueChanged );
    //g_pPanelWatch->AddVector3( "Rot", &m_Rotation, 0, 360, this, ComponentTransform::StaticOnValueChanged );

    if( m_CustomLeftClickCallback )
    {
        m_CustomLeftClickCallback( m_CustomLeftClickObject );
    }
}

void MyFileObject::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    menu.Append( 1000, "Open file" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&MyFileObject::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void MyFileObject::OnPopupClick(wxEvent &evt)
{
    MyFileObject* pFileObject = (MyFileObject*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    int id = evt.GetId();
    if( id == 1000 )
    {
        char url[MAX_PATH];
        char workingdir[MAX_PATH];
        _getcwd( workingdir, MAX_PATH * sizeof(char) );
        sprintf_s( url, MAX_PATH, "%s/%s", workingdir, pFileObject->m_FullPath );
        ShellExecuteA( 0, 0, url, 0, 0, SW_SHOWNORMAL );
    }
}

void MyFileObject::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_FileObjectPointer;
    g_DragAndDropStruct.m_Value = this;
}

void MyFileObject::SetCustomLeftClickCallback(PanelObjectListCallback callback, void* object)
{
    m_CustomLeftClickObject = object;
    m_CustomLeftClickCallback = callback;
}
#endif //MYFW_USING_WX

void MyFileObject::GenerateNewFullPathFilenameInSameFolder(char* newfilename, char* buffer, int buffersize)
{
    assert( buffer != 0 );
    sprintf_s( buffer, buffersize, "%s", m_FullPath );
    int endoffolderoffset = strlen(m_FullPath) - strlen(m_FilenameWithoutExtension) - strlen(m_ExtensionWithDot);
    sprintf_s( &buffer[endoffolderoffset], buffersize - endoffolderoffset, "%s", newfilename );
}

void MyFileObject::GenerateNewFullPathExtensionWithSameNameInSameFolder(char* newextension, char* buffer, int buffersize)
{
    assert( buffer != 0 );
    sprintf_s( buffer, buffersize, "%s", m_FullPath );
    int endoffilenameoffset = strlen(m_FullPath) - strlen(m_ExtensionWithDot);
    sprintf_s( &buffer[endoffilenameoffset], buffersize - endoffilenameoffset, "%s", newextension );
}

void MyFileObject::RequestFile(const char* filename)
{
    assert( filename != 0 );
    if( filename == 0 )
        return;

    LOGInfo( LOGTag, "RequestFile %s\n", filename );

    int len = (int)strlen( filename );
    assert( len > 0 );
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
    if( m_FileLoadStatus != FileLoadStatus_Success && m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading == 0 )
    {
        int length = 0;

        char* buffer = PlatformSpecific_LoadFile( m_FullPath, &length, m_FullPath, __LINE__ );

        if( buffer == 0 )
        {
            m_FileLoadStatus = FileLoadStatus_Error_FileNotFound; // file not found.
            return;
        }

        if( length > 0 && buffer != 0 )
            FakeFileLoad( buffer, length );

#if MYFW_WINDOWS
        WIN32_FIND_DATAA data;
        GetFileData( m_FullPath, &data );

        m_FileLastWriteTime = data.ftLastWriteTime;
#endif
    }

    if( m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading > 0 )
        m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading--;
}

bool MyFileObject::IsNewVersionAvailable()
{
    bool updated = false;
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
    }

    m_FileLastWriteTime = data.ftLastWriteTime;
#endif

    return updated;
}

void MyFileObject::FakeFileLoad(char* buffer, int length)
{
    assert( buffer != 0 && length > 0 );
    if( buffer == 0 || length <= 0 )
        return;

    m_pBuffer = buffer;
    m_FileLength = length;
    m_BytesRead = length;
    m_FileLoadStatus = FileLoadStatus_Success;
}

void MyFileObject::UnloadContents()
{
    SAFE_DELETE_ARRAY( m_pBuffer );
    m_FileLength = 0;
    m_BytesRead = 0;
    m_FileLoadStatus = FileLoadStatus_Loading;

#if _DEBUG
    m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading = 2;
#else
    m_Hack_TicksToWaitUntilWeActuallyLoadToSimulateAsyncLoading = 0;
#endif

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveFile( this );
#endif
}
