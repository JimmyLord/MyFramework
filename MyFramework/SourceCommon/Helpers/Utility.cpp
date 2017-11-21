//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

// Replace all slashes with forward slashes.
void FixSlashesInPath(char* path)
{
    unsigned int i=0;
    while( path[i] != 0 )
    {
        if( path[i] == '\\' )
            path[i] = '/';

        i++;
    }
}

// Will replace backslashes with forward slashes in fullpath.
// Will return 0 if path is not relative.
const char* GetRelativePath(char* fullpath)
{
    char workingdir[MAX_PATH];
#if MYFW_WINDOWS
    GetCurrentDirectoryA( MAX_PATH, workingdir );
#else
    MyAssert( false ); // TODO: implement on OSX and Linux (others?)
    workingdir[0] = 0;
#endif

    unsigned int workingdirpathlen = (unsigned int)strlen( workingdir );
    unsigned int fullpathlen = (unsigned int)strlen( fullpath );

    FixSlashesInPath( workingdir );
    FixSlashesInPath( fullpath );

    // Case insensitive string compare.
    // TODO: Change to strncasecmp on other systems when this doesn't compile.
    if( _strnicmp( workingdir, fullpath, workingdirpathlen ) == 0 )
    {
        return &fullpath[workingdirpathlen+1];
    }

    return 0;
}

void GetFullPath(const char* relativepath, char* fullpath, unsigned int maxcharsinfullpatharray)
{
    char workingdir[MAX_PATH];
#if MYFW_WINDOWS
    GetCurrentDirectoryA( MAX_PATH, workingdir );
#else
    MyAssert( false ); // TODO: implement on OSX and Linux (others?)
    workingdir[0] = 0;
#endif

    sprintf_s( fullpath, maxcharsinfullpatharray, "%s/%s", workingdir, relativepath );
    MyAssert( strlen(fullpath)+1 < maxcharsinfullpatharray );

    FixSlashesInPath( fullpath );
}

void ParseFilename(const char* fullpath, char* outFilename, int sizeFilename, char* outExtension, int sizeExtension)
{
    int len = (int)strlen( fullpath );
    MyAssert( len > 0 );
    if( len <= 0 )
        return;

    int extensionstartlocation = len;
    {
        while( extensionstartlocation > 0 )
        {
            if( fullpath[extensionstartlocation] == '.' )
            {
                int extensionlen = len-extensionstartlocation;
                strncpy_s( outExtension, sizeExtension, &fullpath[extensionstartlocation], extensionlen );
                outExtension[extensionlen] = 0;
                break;
            }
            extensionstartlocation--;
        }

        if( outExtension == 0 )
        {
            outExtension[0] = '.';
            outExtension[1] = 0;

            extensionstartlocation = len;
        }
    }

    {
        int i = extensionstartlocation;
        while( i >= 0 )
        {
            if( i == 0 || fullpath[i] == '/' || fullpath[i] == '\\' )
            {
                if( fullpath[i] == '/' || fullpath[i] == '\\' )
                    i++;
                int namelen = extensionstartlocation-i;
                strncpy_s( outFilename, sizeFilename, &fullpath[i], namelen );
                outFilename[namelen] = 0;
                break;
            }
            i--;
        }
    }
}
