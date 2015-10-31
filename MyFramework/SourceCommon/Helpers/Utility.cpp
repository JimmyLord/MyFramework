//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

// will replace backslashes with forward slashes in fullpath
// will return 0 if path is not relative.
const char* GetRelativePath(char* fullpath)
{
    char workingdir[MAX_PATH];
#if MYFW_WINDOWS
    GetCurrentDirectoryA( MAX_PATH, workingdir );
#else
    workingdir[0] = 0;
#endif

    unsigned int workingdirpathlen = (unsigned int)strlen( workingdir );
    unsigned int fullpathlen = (unsigned int)strlen( fullpath );

    if( strncmp( workingdir, fullpath, workingdirpathlen ) == 0 )
    {
        for( unsigned int i=workingdirpathlen+1; i<fullpathlen-1; i++ )
        {
            if( fullpath[i] == '\\' )
                fullpath[i] = '/';
        }

        return &fullpath[workingdirpathlen+1];
    }

    return 0;
}

const char* GetRelativePath(const char* fullpath)
{
    char workingdir[MAX_PATH];
#if MYFW_WINDOWS
    GetCurrentDirectoryA( MAX_PATH, workingdir );
#else
    workingdir[0] = 0;
#endif

    unsigned int workingdirpathlen = (unsigned int)strlen( workingdir );
    unsigned int fullpathlen = (unsigned int)strlen( fullpath );

    if( strncmp( workingdir, fullpath, workingdirpathlen ) == 0 )
    {
        return &fullpath[workingdirpathlen+1];
    }

    return 0;
}
