//
// Copyright (c) 2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "FileChangeDetector.h"

FileChangeDetector::FileChangeDetector(const char* folderToWatch)
{
#if MYFW_WINDOWS
    m_DirectoryHandle = 0;
    m_pBuffer = nullptr;
    memset( &m_Overlapped, 0, sizeof( m_Overlapped ) );

    char path[MAX_PATH];

    if( folderToWatch == nullptr )
    {
        GetCurrentDirectory( MAX_PATH, path );
    }
    else
    {
        strcpy_s( path, MAX_PATH, folderToWatch );
    }

    // For more info, read https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-readdirectorychangesw
    // and https://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw_19.html
    m_DirectoryHandle = CreateFile( path, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr );

    if( m_DirectoryHandle == INVALID_HANDLE_VALUE )
    {
        LOGError( LOGTag, "Error with CreateFile in FileChangeDetector.\n" );
        m_DirectoryHandle = 0;
    }
    else
    {
        m_Overlapped.hEvent = this;

        m_BufferSize = 1000;
        m_pBuffer = MyNew unsigned char[m_BufferSize];

        WatchForNextFileSystemChange();
    }
#endif
}

FileChangeDetector::~FileChangeDetector()
{
#if MYFW_WINDOWS
    delete[] m_pBuffer;

    CloseHandle( m_DirectoryHandle );
#endif
}

#if MYFW_WINDOWS
void FileChangeDetector::WatchForNextFileSystemChange()
{
    DWORD bytesWritten = 0;

    bool succeeded = ReadDirectoryChangesW( m_DirectoryHandle, m_pBuffer, m_BufferSize, true,
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
        &bytesWritten, &m_Overlapped, &NotificationCompletion );

    if( succeeded == false )
    {
        DWORD error = GetLastError();
        char* pMessage;
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&pMessage, 0, nullptr );
        LOGError( LOGTag, "Error with ReadDirectoryChangesW in FileChangeDetector: %d: %s", error, pMessage );
        LocalFree( pMessage );
    }
}

// Static.
void CALLBACK FileChangeDetector::NotificationCompletion(DWORD errorCode, DWORD numBytesTransfered, LPOVERLAPPED pOverlapped)
{
    if( errorCode != ERROR_SUCCESS )
    {
        LOGError( LOGTag, "Error with FileChangeDetector: %d\n", errorCode );
    }

    FileChangeDetector* pThis = (FileChangeDetector*)pOverlapped->hEvent;

    FILE_NOTIFY_INFORMATION* pFileInfo = (FILE_NOTIFY_INFORMATION*)pThis->m_pBuffer;
    char name[MAX_PATH];
    size_t count;
    int len = pFileInfo->FileNameLength/2;
    wcstombs_s( &count, name, MAX_PATH, pFileInfo->FileName, len );
    name[len] = 0;
    LOGInfo( LOGTag, "NotificationCompletion: File changed: %s\n", name );

    pThis->WatchForNextFileSystemChange();
}
#endif

void FileChangeDetector::CheckForChanges()
{
    bool somethingChanged = false;

#if MYFW_WINDOWS
    if( m_DirectoryHandle == 0 )
        return;

    DWORD bytesTransferred;
    bool succeeded = GetOverlappedResult( m_DirectoryHandle, &m_Overlapped, &bytesTransferred, true );
    if( succeeded )
    {
        FILE_NOTIFY_INFORMATION* pFileInfo = (FILE_NOTIFY_INFORMATION*)m_pBuffer;
        char name[MAX_PATH];
        size_t count;
        int len = pFileInfo->FileNameLength/2;
        wcstombs_s( &count, name, MAX_PATH, pFileInfo->FileName, len );
        name[len] = 0;
        LOGInfo( LOGTag, "GetOverlappedResult: File changed: %s\n", name );

        WatchForNextFileSystemChange();
    }
#endif

    if( somethingChanged )
    {
        // A file has changed.
        LOGInfo( LOGTag, "Something changed in this directory." );
    }
}