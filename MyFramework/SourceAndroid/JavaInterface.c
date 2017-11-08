//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include <string.h>

#include "../../MyFramework/SourceAndroid/JavaInterfaceCPP.h"
#include "JavaInterface.h"

int g_AppAlive = 1;
JNIEnv* g_pJavaEnvironment = 0;
jobject g_pMainActivity = 0;
jobject g_pAssetManager = 0;
jobject g_pBMPFactoryLoader = 0;
jobject g_pJavaSoundPlayer = 0;
char g_pAndroidDeviceName[128];

static long _getTime(void)
{
    struct timeval now;

    gettimeofday( &now, NULL );
    return (long)(now.tv_sec*1000 + now.tv_usec/1000);
}

// Call to initialize the graphics state
void Java_com_flathead_MYFWPackage_MYFWActivity_NativeOnCreate(JNIEnv* env, jobject thiz, jobject activity, jobject assetmgr, jobject bmploader, jobject soundplayer, jstring devicename, jstring launchscene)
{
    //LOGInfo( LOGTag, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    //LOGInfo( LOGTag, ">");
    //LOGInfo( LOGTag, "> Starting new run");
    //LOGInfo( LOGTag, ">");
    //LOGInfo( LOGTag, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

    __android_log_print( ANDROID_LOG_INFO, "Flathead", "[Flow] - NativeOnCreate" );

    g_pJavaEnvironment = env;
    g_pMainActivity = activity;
    g_pAssetManager = assetmgr;
    g_pBMPFactoryLoader = bmploader;
    g_pJavaSoundPlayer = soundplayer;

    const char* devicenamechar = (*env)->GetStringUTFChars( env, devicename, 0 );
    int len = strlen(devicenamechar);
    if( len > 127 )
        len = 127;
    int i;
    for( i=0; i<len; i++ )
        g_pAndroidDeviceName[i] = devicenamechar[i];
    g_pAndroidDeviceName[i] = 0;
    (*env)->ReleaseStringUTFChars( env, devicename, devicenamechar );

    const char* launchscenebuffer[260];
    launchscenebuffer[0] = 0;

    if( launchscene )
    {
        const char* launchscenechar = (*env)->GetStringUTFChars( env, launchscene, 0 );
        strcpy( launchscenebuffer, launchscenechar );
        (*env)->ReleaseStringUTFChars( env, launchscene, launchscenechar );

        __android_log_print( ANDROID_LOG_INFO, "Flathead", launchscenebuffer );
    }

    {
        App_Activity_OnCreate( launchscenebuffer );
    }

    g_pJavaEnvironment = 0;
    g_pMainActivity = 0;
    g_pAssetManager = 0;
    g_pBMPFactoryLoader = 0;
    g_pJavaSoundPlayer = 0;
}

void Java_com_flathead_MYFWPackage_MYFWActivity_NativeOnDestroy(JNIEnv* env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_INFO, "Flathead", "[Flow] - NativeOnDestroy");

    App_Activity_OnDestroy();
}

// Call to initialize the graphics state
void Java_com_flathead_MYFWPackage_MYFWActivity_NativeOnBackPressed(JNIEnv* env, jobject thiz, jobject activity, jobject assetmgr, jobject bmploader, jobject soundplayer)
{
    __android_log_print(ANDROID_LOG_INFO, "Flathead", "[Flow] - NativeOnBackPressed");

    g_pJavaEnvironment = env;
    g_pMainActivity = activity;
    g_pAssetManager = assetmgr;
    g_pBMPFactoryLoader = bmploader;
    g_pJavaSoundPlayer = soundplayer;

    {
        App_Activity_OnBackPressed();
    }

    g_pJavaEnvironment = 0;
    g_pMainActivity = 0;
    g_pAssetManager = 0;
    g_pBMPFactoryLoader = 0;
    g_pJavaSoundPlayer = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Java_com_flathead_MYFWPackage_MyGL2SurfaceView_NativeOnTouchEvent(JNIEnv* env, jobject thiz,
    jobject activity, jobject assetmgr, jobject bmploader, jobject soundplayer,
    int action, int actionindex, int actionmasked, int tool, int id, float x, float y, float pressure, float size)
{
    //g_pJavaEnvironment = env;
    //g_pMainActivity = activity;
    //g_pAssetManager = assetmgr;
    //g_pBMPFactoryLoader = bmploader;
    //g_pJavaSoundPlayer = soundplayer;

    {
        App_GLSurfaceView_OnTouch( action, actionindex, actionmasked, tool, id, x, y, pressure, size );
    }

    //g_pJavaEnvironment = 0;
    //g_pMainActivity = 0;
    //g_pAssetManager = 0;
    //g_pBMPFactoryLoader = 0;
    //g_pJavaSoundPlayer = 0;
    //__android_log_print(ANDROID_LOG_INFO, "Flathead", "NativeOnTouchEvent (%d %d %d)(%d %d)(%f,%f)(%f %f)", action, actionindex, actionmasked, tool, id, x, y, pressure, size);
}

void Java_com_flathead_MYFWPackage_MyGL2Renderer_NativeOnSurfaceCreated()
{
    __android_log_print(ANDROID_LOG_INFO, "Flathead", "[Flow] - NativeOnSurfaceCreated");

    App_GLRenderer_SurfaceCreated();

    g_AppAlive = 1;
}

void Java_com_flathead_MYFWPackage_MyGL2Renderer_NativeOnSurfaceChanged(JNIEnv* env, jobject thiz, jint w, jint h)
{
    __android_log_print(ANDROID_LOG_INFO, "Flathead", "[Flow] - NativeOnSurfaceChanged (%dx%d)", w, h);

    App_GLRenderer_SurfaceChanged( w, h );
}

// Call to render the next GL frame
void Java_com_flathead_MYFWPackage_MyGL2Renderer_NativeRender(JNIEnv* env, jobject thiz, jobject activity, jobject assetmgr, jobject bmploader, jobject soundplayer)
{
    //__android_log_print(ANDROID_LOG_INFO, "Flathead", "[Flow] - NativeRender");

    g_pJavaEnvironment = env;
    g_pMainActivity = activity;
    g_pAssetManager = assetmgr;
    g_pBMPFactoryLoader = bmploader;
    g_pJavaSoundPlayer = soundplayer;

    {
        long time = _getTime();

        App_GLRenderer_NativeRender( time );
    }

    g_pJavaEnvironment = 0;
    g_pMainActivity = 0;
    g_pAssetManager = 0;
    g_pBMPFactoryLoader = 0;
    g_pJavaSoundPlayer = 0;
}

void Java_com_flathead_MYFWPackage_MYFWActivity_NativeOnKeyDown(JNIEnv* env, jobject thiz, int keycode, int unicodechar, jobject activity, jobject assetmgr, jobject bmploader, jobject soundplayer)
{
    __android_log_print(ANDROID_LOG_INFO, "Flathead", "NativeOnKeyDown");

    //g_pJavaEnvironment = env;
    //g_pMainActivity = activity;
    //g_pAssetManager = assetmgr;
    //g_pBMPFactoryLoader = bmploader;
    //g_pJavaSoundPlayer = soundplayer;

    {
        App_Activity_OnKeyDown( keycode, unicodechar );
    }

    //g_pJavaEnvironment = 0;
    //g_pMainActivity = 0;
    //g_pAssetManager = 0;
    //g_pBMPFactoryLoader = 0;
    //g_pJavaSoundPlayer = 0;

    __android_log_print(ANDROID_LOG_INFO, "Flathead", "~NativeOnKeyDown");
}

void Java_com_flathead_MYFWPackage_MYFWActivity_NativeOnKeyUp(JNIEnv* env, jobject thiz, int keycode, int unicodechar, jobject activity, jobject assetmgr, jobject bmploader, jobject soundplayer)
{
    //__android_log_print(ANDROID_LOG_INFO, "Flathead", "NativeOnKeyUp");

    //g_pJavaEnvironment = env;
    //g_pMainActivity = activity;
    //g_pAssetManager = assetmgr;
    //g_pBMPFactoryLoader = bmploader;
    //g_pJavaSoundPlayer = soundplayer;

    {
        App_Activity_OnKeyUp( keycode, unicodechar );
    }

    //g_pJavaEnvironment = 0;
    //g_pMainActivity = 0;
    //g_pAssetManager = 0;
    //g_pBMPFactoryLoader = 0;
    //g_pJavaSoundPlayer = 0;
}

void Java_com_flathead_MYFWPackage_IAPManager_NativeOnResult(JNIEnv* env, jobject thiz,
                       jint responseCode, jstring jpurchaseData, jstring jdataSignature,
                       jstring jsku, jstring jpayload)
{
    __android_log_print(ANDROID_LOG_INFO, "Flathead", "IAPManager::NativeOnResult");

    const char* purchaseData  = (*env)->GetStringUTFChars( env, jpurchaseData, 0 );
    const char* dataSignature = (*env)->GetStringUTFChars( env, jdataSignature, 0 );
    const char* sku           = (*env)->GetStringUTFChars( env, jsku, 0 );
    const char* payload       = (*env)->GetStringUTFChars( env, jpayload, 0 );

    App_IAPManager_OnResult( responseCode, purchaseData, dataSignature, sku, payload );
}
