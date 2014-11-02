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

#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include <pthread.h>

#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <GLES2/gl2.h>
#if !ANDROID_NDK
#include <GLES2/egl2.h>
#endif // !ANDROID_NDK

#include "JavaInterfaceCPP.h"
#include "../SourceCommon/CommonHeader.h"

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

void App_Activity_OnCreate()
{
    AndroidMain_CreateGameCore();

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

// Called from the app framework.
void App_GL_OnSurfaceCreated()
{
    if( g_pGameCore )
    {
        g_pGameCore->OnSurfaceCreated();
    }
}

void App_GL_OnSurfaceChanged(int w, int h)
{
    glViewport(0, 0, w, h);

    if( g_pGameCore )
    {
        g_pGameCore->OnSurfaceChanged( 0, 0, w, h );
    }
}

// Called from the app framework.
void appDeinit2()
{
    // TODO: write a better shutdown?
    g_pGameCore->OnSurfaceLost();
}

void appSurfaceLost()
{
    g_pGameCore->OnSurfaceLost();
}

void appOnKeyDown(int keycode, int unicodechar)
{
    g_pGameCore->OnKeyDown( keycode, unicodechar );
}

void appOnKeyUp(int keycode, int unicodechar)
{
    g_pGameCore->OnKeyUp( keycode, unicodechar );
}

void appOnTouch(int action, int actionindex, int actionmasked, int tool, int id, float x, float y, float pressure, float size)
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

// Called from the app framework.
// The tick is current time in milliseconds, width and height
// are the image dimensions to be rendered.
void appRender2(long tick, int width, int height)
{
    if( !g_AppAlive )
        return;

    if( g_pGameCore->m_OneTimeInitWasCalled == false )
        g_pGameCore->OneTimeInit();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    static long lastmills = tick;
    long millspassed = tick - lastmills;
    //LOGInfo( LOGTag, "mills last(%d) tick(%d) diff(%d)", (int)lastmills, (int)tick, (int)millspassed );
    lastmills = tick;

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
            g_pGameCore->OnButtons( (GameCoreButtonActions)event->action, (GameCoreButtonIDs)event->id );
        }

        currentreadindex++;
        if( currentreadindex == MAX_TOUCH_EVENTS )
            currentreadindex = 0;
    }
    pthread_mutex_unlock( &g_TouchInputMutex );

    g_pGameCore->Tick(millspassed / 1000.0f);

    g_pGameCore->OnDrawFrame();
    g_pGameCore->OnDrawFrameDone();

    //DrawScene();
}
