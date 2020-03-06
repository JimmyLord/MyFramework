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
    char folder[MAX_PATH];

    if( folderToWatch == nullptr )
    {
        GetCurrentDirectory( MAX_PATH, folder );
    }
    else
    {
        strcpy_s( folder, MAX_PATH, folderToWatch );
    }

    m_ChangeHandle = FindFirstChangeNotification( folder, true, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE );

    if( m_ChangeHandle == INVALID_HANDLE_VALUE )
    {
        LOGError( LOGTag, "File change handle failed to initialize.\n" );
    }
#endif
}

FileChangeDetector::~FileChangeDetector()
{
#if MYFW_WINDOWS
    FindCloseChangeNotification( m_ChangeHandle );
#endif
}

void FileChangeDetector::CheckForChanges()
{
    bool somethingChanged = false;

#if MYFW_WINDOWS
    DWORD waitStatus;

    do
    {
        waitStatus = WaitForSingleObject( m_ChangeHandle, 0 );

        if( waitStatus == WAIT_OBJECT_0 )
        {
            FindNextChangeNotification( m_ChangeHandle );

            somethingChanged = true;
        }
    }
    while( waitStatus == WAIT_OBJECT_0 );
#endif

    if( somethingChanged )
    {
        // A file has changed.
        LOGInfo( LOGTag, "Something changed in this directory." );
    }
}