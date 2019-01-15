//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"
#include <stdint.h>
#include <stdio.h>

#if MYFW_OSX
#include <Foundation/NSPathUtilities.h>
#endif
#include <Foundation/NSFileManager.h>

#include <Corefoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>

//- (NSURL *)applicationDocumentsDirectory
//{
//    return [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
//}

FILE* OpenSavedDataFile(const char* subpath, const char* filename, const char* filemode)
{
    NSString* path = 0;
    FILE* file = 0;

    NSURL* documentURL = [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
    //NSArray* documentDir = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES );

    //if( documentDir )
    //{
    //    path = [documentDir objectAtIndex:0];
    if( documentURL )
    {
        path = [documentURL path];

        const char* approotpathstr = [path cStringUsingEncoding:NSASCIIStringEncoding];

        char fullpath[PATH_MAX];
        sprintf_s( fullpath, PATH_MAX, "%s/%s%s", approotpathstr, subpath, filename );

        file = fopen( fullpath, filemode );
    }

    return file;
}
