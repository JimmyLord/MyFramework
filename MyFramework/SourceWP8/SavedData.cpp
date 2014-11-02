//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
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

using namespace Windows::Storage;
using namespace Windows::ApplicationModel;

FILE* OpenSavedDataFile(const char* subpath, const char* filename, const char* filemode)
{
    wchar_t wide_subpath[MAX_PATH];
    wchar_t wide_filename[MAX_PATH];
    wchar_t wide_filemode[10];
    wchar_t wide_fullpath[MAX_PATH];
    
    size_t numconverted;
    mbstowcs_s( &numconverted, wide_subpath, subpath, MAX_PATH );
    mbstowcs_s( &numconverted, wide_filename, filename, MAX_PATH );
    mbstowcs_s( &numconverted, wide_filemode, filemode, 10 );

	StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
	Platform::String^ folderPath = localFolder->Path;
    swprintf_s( wide_fullpath, MAX_PATH, L"%s\\%s\\%s", folderPath->Data(), wide_subpath, wide_filename );

    FILE* file;
    errno_t err = _wfopen_s( &file, wide_fullpath, wide_filemode );

    return file;
}
