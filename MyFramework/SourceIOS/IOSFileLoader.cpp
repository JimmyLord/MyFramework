//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"
#include "IOSFileLoader.h"
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>

FILE* IOS_fopen(const char* filepathnameext)
{
    // TODO: uncomment next line
    //return g_pFileManager->RequestFile( filename );

    //LOGInfo( LOGTag, "OLD FASHIONED RequestFile %s\n", filepathnameext );

    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if( mainBundle == 0 )
        return 0;
    
    char filedir[MAX_PATH];
    char filename[MAX_PATH];
    char fileext[MAX_PATH];

    char filecopy[MAX_PATH];
    strcpy_s( filecopy, MAX_PATH, filepathnameext );
    
    int len = (int)strlen( filecopy );
    int i;
    for( i=len-1; i>=0; i-- )
    {
        if( filecopy[i] == '.' )
        {
            strcpy_s( fileext, MAX_PATH, &filecopy[i+1] );
            filecopy[i] = 0;
            break;
        }
    }
    for( ; i>=0; i-- )
    {
        if( filecopy[i] == '/' )
        {
            strcpy_s( filename, MAX_PATH, &filecopy[i+1] );
            filecopy[i] = 0;
            break;
        }
    }
    strcpy_s( filedir, MAX_PATH, &filecopy[0] );
    
    CFStringRef filenameref = CFStringCreateWithCString( kCFAllocatorDefault, filename, kCFStringEncodingMacRoman );
    CFStringRef fileextref = CFStringCreateWithCString( kCFAllocatorDefault, fileext, kCFStringEncodingMacRoman );
    CFStringRef filedirref = CFStringCreateWithCString( kCFAllocatorDefault, filedir, kCFStringEncodingMacRoman );

    CFURLRef fileurl = CFBundleCopyResourceURL( mainBundle, filenameref, fileextref, filedirref);
    
    CFRelease( filenameref );
    CFRelease( fileextref );
    CFRelease( filedirref );

    if( fileurl == 0 )
        return 0;
    
    UInt8 filefullpath[1024];
    CFURLGetFileSystemRepresentation( fileurl, true,
                                      filefullpath, sizeof(filefullpath) );
    CFRelease(fileurl);
    
    if( filefullpath[0] == 0 )
        return 0;
        
    FILE* filehandle;
    filehandle = fopen( (char*)filefullpath, "rb" );

    return filehandle;
}
