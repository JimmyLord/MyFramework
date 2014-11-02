//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include <stdint.h>
#include <stdio.h>

FILE* OpenSavedDataFile(const char* subpath, const char* filename, const char* filemode)
{
    char* pathstr = "data";

    char fullpath[MAX_PATH];
    sprintf_s( fullpath, MAX_PATH, "%s/%s%s", pathstr, subpath, filename );

    FILE* file;
#if MYFW_WINDOWS
    errno_t err = fopen_s( &file, fullpath, filemode );
#else
    file = fopen( fullpath, filemode );
#endif

    return file;
}
