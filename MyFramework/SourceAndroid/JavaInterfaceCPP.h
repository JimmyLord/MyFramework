//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __JavaInterfaceCPP_H__
#define __JavaInterfaceCPP_H__

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

extern void App_Activity_OnCreate();
extern void App_Activity_OnDestroy();
extern void App_Activity_OnPause();
extern void App_Activity_OnResume();
extern void App_Activity_OnBackPressed();
extern void App_Activity_OnKeyDown(int keycode, int unicodechar);
extern void App_Activity_OnKeyUp(int keycode, int unicodechar);

extern void App_GLSurfaceView_OnTouch(int action, int actionindex, int actionmasked, int tool, int id, float x, float y, float pressure, float size);

extern void App_GLSurfaceView_SurfaceCreated();
extern void App_GLSurfaceView_SurfaceChanged(int w, int h);
extern void App_GLSurfaceView_SurfaceDestroyed();

extern void App_GLRenderer_NativeRender(long currenttimemilliseconds);

extern void App_IAPManager_OnResult(int responseCode, const char* purchaseData, const char* dataSignature, const char* sku, const char* payload);

#ifdef __cplusplus
}
#endif

// the AndroidMain_CreateGameCore() should be defined in main.cpp and
//     create a new instance of the game specific subclass of GameCore.
void AndroidMain_CreateGameCore();

#endif //__JavaInterfaceCPP_H__
