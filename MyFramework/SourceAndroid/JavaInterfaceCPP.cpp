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
#include <pthread.h>

#include <stdlib.h>
#include <math.h>

#include <GLES2/gl2.h>

#include "../SourceCommon/MyFrameworkPCH.h"
#include "JavaInterfaceCPP.h"
#include "Core/GameCore.h"
#include "Events/EventManager.h"
#include "Events/EventTypeManager.h"

//#if !ANDROID_NDK
//#include <GLES2/egl2.h>
//#endif // !ANDROID_NDK

bool g_Android_ExitOnBackButton = true;
extern double g_UnpausedTime;

struct AndroidTouchEvent
{
    int eventtype; // 0 for touch, 1 for button
    
    int action;
    int actionindex;
    int actionmasked;
    int tool;
    int id;
    float x;
    float y;
    float pressure;
    float size;
};

#define MAX_TOUCH_EVENTS    500

pthread_mutex_t g_TouchInputMutex;
AndroidTouchEvent g_TouchEventQueue[MAX_TOUCH_EVENTS];
int currentreadindex = 0;
int currentwriteindex = 0;

void App_Activity_OnCreate(const char* launchScene)
{
    AndroidMain_CreateGameCore( launchScene );

    pthread_mutex_init( &g_TouchInputMutex, 0 );
}

void App_Activity_OnDestroy()
{
    delete g_pGameCore;
    g_pGameCore = 0;

    pthread_mutex_destroy( &g_TouchInputMutex );
}

void App_Activity_OnBackPressed()
{
    pthread_mutex_lock( &g_TouchInputMutex );

    //LOGInfo( LOGTag, "OnTouch (%d %d %d)(%d %d)(%f,%f)(%f %f)", action, actionindex, actionmasked, tool, id, x, y, pressure, size);

    // queue up touch events into circular buffer.

    int onebelowreadindex = currentreadindex-1;
    if( onebelowreadindex == -1 )
        onebelowreadindex = MAX_TOUCH_EVENTS-1;

    if( currentwriteindex == onebelowreadindex )
    {
        // there's room for one more, but we're not going to use it to make comparing read and write index possible.
        LOGInfo( LOGTag, "WARNING: Throwing out touch events: queue is full" );
        return;
    }

    //LOGInfo( LOGTag, "Writing Touch event: %d\n", currentwriteindex );

    AndroidTouchEvent* event = &g_TouchEventQueue[currentwriteindex];
    currentwriteindex++;
    if( currentwriteindex == MAX_TOUCH_EVENTS )
        currentwriteindex = 0;

    event->eventtype = 1;
    event->action = GCBA_Down;
    event->id = GCBI_Back;
   
    pthread_mutex_unlock( &g_TouchInputMutex );
}

void App_Activity_OnKeyDown(int keycode, int unicodechar)
{
    g_pGameCore->OnKeyDown( keycode, unicodechar );
}

void App_Activity_OnKeyUp(int keycode, int unicodechar)
{
    g_pGameCore->OnKeyUp( keycode, unicodechar );
}

void App_GLSurfaceView_OnTouch(int action, int actionindex, int actionmasked, int tool, int id, float x, float y, float pressure, float size)
{
    pthread_mutex_lock( &g_TouchInputMutex );

    //LOGInfo( LOGTag, "OnTouch (%d %d %d)(%d %d)(%f,%f)(%f %f)", action, actionindex, actionmasked, tool, id, x, y, pressure, size);

    // queue up touch events into circular buffer.

    int onebelowreadindex = currentreadindex-1;
    if( onebelowreadindex == -1 )
        onebelowreadindex = MAX_TOUCH_EVENTS-1;

    if( currentwriteindex == onebelowreadindex )
    {
        // there's room for one more, but we're not going to use it to make comparing read and write index possible.
        LOGInfo( LOGTag, "WARNING: Throwing out touch events: queue is full" );
        return;
    }

    //LOGInfo( LOGTag, "Writing Touch event: %d\n", currentwriteindex );

    AndroidTouchEvent* event = &g_TouchEventQueue[currentwriteindex];
    currentwriteindex++;
    if( currentwriteindex == MAX_TOUCH_EVENTS )
        currentwriteindex = 0;

    event->eventtype = 0;
    event->action = action;
    event->actionindex = actionindex;
    event->actionmasked = actionmasked;
    event->tool = tool;
    event->id = id;
    event->x = x;
    event->y = y;
    event->pressure = pressure;
    event->size = size;
   
    pthread_mutex_unlock( &g_TouchInputMutex );
}

void App_GLRenderer_SurfaceCreated()
{
    if( g_pGameCore )
    {
        // in theory should get called only the first time the context is created
        // or after resuming if the context was destroyed
        // so, invalidate all the gl objects and rebuild them later.
        g_pGameCore->OnSurfaceLost();
        g_pGameCore->OnSurfaceCreated();
    }
}

void App_GLRenderer_SurfaceChanged(int w, int h)
{
    glViewport( 0, 0, w, h );

    if( g_pGameCore )
    {
        g_pGameCore->OnSurfaceChanged( 0, 0, w, h );
    }
}

void App_GLRenderer_NativeRender(long currenttimemilliseconds)
{
    if( !g_AppAlive )
        return;

    if( g_pGameCore->HasOneTimeInitBeenCalled() == false )
        g_pGameCore->OneTimeInit();

    static long lastmills = currenttimemilliseconds;
    long millspassed = currenttimemilliseconds - lastmills;
    //LOGInfo( LOGTag, "mills last(%d) tick(%d) diff(%d)", (int)lastmills, (int)tick, (int)millspassed );
    lastmills = currenttimemilliseconds;

    static int numframes = 0;
    static int totaltime = 0;

    totaltime += millspassed;
    numframes++;
    if( totaltime > 1000 * 10 )
    {
        //LOGInfo( LOGTag, "fps - totaltime(%d) - frames(%d)", totaltime, numframes/10 );
        numframes = 0;
        totaltime = 0;
    }

    // process queued up touch events:
    pthread_mutex_lock( &g_TouchInputMutex );
    while( currentreadindex != currentwriteindex )
    {
        AndroidTouchEvent* event = &g_TouchEventQueue[currentreadindex];

        //LOGInfo( LOGTag, "Reading input event: %d, type: %d\n", currentreadindex, event->eventtype );

        if( event->eventtype == 0 )
        {
            //LOGInfo( LOGTag, "g_pGameCore->OnTouch: %d\n", currentreadindex );

            if( event->actionmasked == 5 ) // convert 2nd+ finger down message to finger down message
            {
                event->id = event->actionindex;
                event->actionmasked = 0;
            }
            if( event->actionmasked == 6 ) // convert 2nd+ finger up message to finger up message
            {
                event->id = event->actionindex;
                event->actionmasked = 1;
            }

            g_pGameCore->OnTouch( event->actionmasked, event->id, event->x, event->y, event->pressure, event->size );
        }
        else //if( event->eventtype == 1 )
        {
            if( g_Android_ExitOnBackButton &&
                event->id == GCBI_Back && event->action == GCBA_Down )
            {
                LOGInfo( LOGTag, "[Flow] Calling Java finish() from C++\n" );
                jclass cls = g_pJavaEnvironment->GetObjectClass( g_pMainActivity );
                jmethodID finish = g_pJavaEnvironment->GetMethodID( cls, "finish", "()V" );
                g_pJavaEnvironment->CallVoidMethod( g_pMainActivity, finish );

                // throw away all other input events and return;
                currentreadindex = currentwriteindex;
                pthread_mutex_unlock( &g_TouchInputMutex );
                return;
            }

            g_pGameCore->OnButtons( (GameCoreButtonActions)event->action, (GameCoreButtonIDs)event->id );
        }

        currentreadindex++;
        if( currentreadindex == MAX_TOUCH_EVENTS )
            currentreadindex = 0;
    }
    pthread_mutex_unlock( &g_TouchInputMutex );

    g_pGameCore->OnDrawFrameStart( 0 );
    g_UnpausedTime += g_pGameCore->Tick( millspassed / 1000.0f );
    g_pGameCore->OnDrawFrame( 0 );
    g_pGameCore->OnDrawFrameDone();
}

void App_IAPManager_OnResult(int responseCode, const char* purchaseData, const char* dataSignature,
                             const char* sku, const char* payload)
{
    EventManager* pEventManager = g_pGameCore->GetManagers()->GetEventManager();
    MyEvent* pIAPEvent = pEventManager->CreateNewEvent( Event_IAP );

    if( pIAPEvent )
    {
        //int responseCode = event->GetInt( "responseCode" );
        //const char* sku = (const char*)event->GetPointer( "sku" );
        //const char* payload = (const char*)event->GetPointer( "payload" );

        pIAPEvent->AttachInt(     pEventManager,      "responseCode", responseCode        );

        pIAPEvent->AttachPointer( pEventManager,      "sku",          (void*)sku          );
        pIAPEvent->AttachPointer( pEventManager,      "payload",      (void*)payload      );

        LOGInfo( LOGTag, "Sending IAPEvent\n" );
        pEventManager->SendEventNow( pIAPEvent );
    }
}
