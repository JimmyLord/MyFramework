//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "../SourceCommon/CommonHeader.h"
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include <stdio.h>

#include "UnthoughtOutAndroidHeader.h"

void StartAdView()
{
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "StartAdView() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return;
    }

    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
    LOGInfo( LOGTag, "StartAdView - cls %p", cls );

    jmethodID StartAdView = g_pJavaEnvironment->GetMethodID( cls, "StartAdView", "()V" );

    LOGInfo( LOGTag, "StartAdView - StartAdView %p", StartAdView );

    if( StartAdView == 0 )
        return;

    g_pJavaEnvironment->CallVoidMethod( g_pMainActivity, StartAdView );
}

FILE* OpenSavedDataFile(const char* subpath, const char* filename, const char* filemode)
{
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "OpenSavedDataFile() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return 0;
    }

    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
    LOGInfo( LOGTag, "OpenSavedDataFile - cls %p", cls );

    jmethodID GetFilesDir = g_pJavaEnvironment->GetMethodID( cls, "GetFilesDir", "()Ljava/io/File;" );

    LOGInfo( LOGTag, "OpenSavedDataFile - GetFilesDir %p", GetFilesDir );

    if( GetFilesDir == 0 )
        return 0;

    jobject dirobj = g_pJavaEnvironment->CallObjectMethod( g_pMainActivity, GetFilesDir );
    jclass dir = g_pJavaEnvironment->GetObjectClass( dirobj );

    LOGInfo( LOGTag, "OpenSavedDataFile - dir %p", dir );

    jmethodID getStoragePath = g_pJavaEnvironment->GetMethodID( dir, "getAbsolutePath", "()Ljava/lang/String;" );
    jstring path = (jstring)g_pJavaEnvironment->CallObjectMethod( dirobj, getStoragePath );

    const char* pathstr = g_pJavaEnvironment->GetStringUTFChars( path, 0 );

    LOGInfo( LOGTag, "OpenSavedDataFile - pathstr %s", pathstr );
    //chdir(pathstr);

    char fullpath[MAX_PATH];
    sprintf_s( fullpath, MAX_PATH, "%s/%s%s", pathstr, subpath, filename );

    g_pJavaEnvironment->ReleaseStringUTFChars( path, pathstr );

    FILE* file = fopen( fullpath, filemode );

    LOGInfo( LOGTag, "OpenSavedDataFile - fullpath %s", fullpath );

    return file;
}
