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

#ifndef __UnthoughtOutAndroidHeader_H__
#define __UnthoughtOutAndroidHeader_H__

/* Value is non-zero when application is alive, and 0 when it is closing.
 * Defined by the application framework.
 */
extern int g_AppAlive;

extern JNIEnv* g_pJavaEnvironment;
extern jobject g_pMainActivity;
extern jobject g_pAssetManager;
extern jobject g_pBMPFactoryLoader;
extern jobject g_pJavaSoundPlayer;
extern char g_pAndroidDeviceName[128];

#ifdef __cplusplus
extern "C" {
#endif

// The simple framework expects the application code to define these functions.
extern void appInit();
extern void appDeinit();
extern void appRender(long tick, int width, int height);

extern void App_Activity_OnCreate();
extern void App_Activity_OnDestroy();
extern void App_Activity_OnPause();
extern void App_Activity_OnResume();
extern void App_Activity_OnBackPressed();

extern void App_GL_OnSurfaceCreated();
extern void App_GL_OnSurfaceChanged(int w, int h);
extern void appDeinit2();
extern void appSurfaceLost();
extern void appRender2(long tick, int width, int height);
extern void appOnKeyDown(int keycode, int unicodechar);
extern void appOnKeyUp(int keycode, int unicodechar);
extern void appOnTouch(int action, int actionindex, int actionmasked, int tool, int id, float x, float y, float pressure, float size);

#ifdef __cplusplus
}
#endif

#endif //__UnthoughtOutAndroidHeader_H__
