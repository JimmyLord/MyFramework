//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
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
#if MYFW_WINDOWS
    char workingdir[MAX_PATH];
    GetCurrentDirectoryA( MAX_PATH, workingdir );
#else
    char workingdir[PATH_MAX];
    getcwd( workingdir, PATH_MAX );
#endif

    unsigned int workingdirpathlen = (unsigned int)strlen( workingdir );
    //unsigned int fullpathlen = (unsigned int)strlen( fullpath );

    FixSlashesInPath( workingdir );
    FixSlashesInPath( fullpath );

    // Case insensitive string compare.
    if( _strnicmp( workingdir, fullpath, workingdirpathlen ) == 0 )
    {
        return &fullpath[workingdirpathlen+1];
    }

    return 0;
}

void GetFullPath(const char* relativepath, char* fullpath, unsigned int maxcharsinfullpatharray)
{
#if MYFW_WINDOWS
    char workingdir[MAX_PATH];
    GetCurrentDirectoryA( MAX_PATH, workingdir );
#else
    char workingdir[PATH_MAX];
    getcwd( workingdir, PATH_MAX );
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

// Taken from https://stackoverflow.com/questions/27303062/strstr-function-like-that-ignores-upper-or-lower-case
//   and changed to accomodate an additional needle terminator as well as 0.
char* MyStrIStr(const char* haystack, const char* needle, unsigned char extraNeedleTerminator)
{
    do
    {
        const char* h = haystack;
        const char* n = needle;
        while( tolower( (unsigned char)*h ) == tolower( (unsigned char)*n ) && n != 0 && *n != extraNeedleTerminator )
        {
            h++;
            n++;
        }
        if( *n == 0 || *n == extraNeedleTerminator )
        {
            return (char*)haystack;
        }
    } while( *haystack++ );
    
    return 0;
}

bool CheckIfMultipleSubstringsAreInString(const char* string, const char* substrings, unsigned char delimiter)
{
    MyAssert( string != 0 );
    MyAssert( substrings != 0 );

    const char* ss = substrings;

    while( *ss != 0 )
    {
        // Advance to next substring after a delimiter. Ignores multiple delimiters in a row.
        while( *ss == delimiter )
        {
            ss++;
        }

        // 'ss' should be at the start of one of the substrings.
        if( *ss != 0 && MyStrIStr( string, ss, delimiter ) == 0 )
        {
            // This substring wasn't found.
            return false;
        }

        // Advance to next delimiter or end of string.
        while( *ss != 0 && *ss != delimiter )
        {
            ss++;
        }
    }
    
    return true;
}
