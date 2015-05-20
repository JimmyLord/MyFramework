//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include <screen/screen.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <input/event_types.h>
#include <input/screen_helpers.h>
#include <sys/keycodes.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <math.h>

#include "bbutil.h"
#include "BBMain.h"

static screen_context_t screen_cxt;

bool h_mouseisdown = false;

#define MAX_KEYS_WE_HANDLE_HELD_EVENTS_FOR  255
bool h_keysheld[MAX_KEYS_WE_HANDLE_HELD_EVENTS_FOR];
bool h_oldkeysheld[MAX_KEYS_WE_HANDLE_HELD_EVENTS_FOR];

int g_BBMain_EGLSwapInterval = 1;

// this is all a fucking ugly mess...
void StoreOldKeyHeldStateAndClearCurrent()
{
    for( int i=0; i<MAX_KEYS_WE_HANDLE_HELD_EVENTS_FOR; i++ )
    {
        h_oldkeysheld[i] = h_keysheld[i];
    }
}

void ClearKeysHeld()
{
    for( int i=0; i<MAX_KEYS_WE_HANDLE_HELD_EVENTS_FOR; i++ )
        h_keysheld[i] = false;
}

void GenerateKeyHeldMessages()
{
    for( int i=0; i<MAX_KEYS_WE_HANDLE_HELD_EVENTS_FOR; i++ )
    {
        if( h_keysheld[i] == true && h_oldkeysheld[i] == true )
            BBFUNC_HandleKeyboardEvents( GCBA_Held, i );
    }
}

void handleNavigatorEvent(bps_event_t *event)
{
    //navigator_event_t nav_event = navigator_event_get_event( event );
    int code = bps_event_get_code( event );

    if( code == NAVIGATOR_LOW_MEMORY )
    {
    }
    else if( code == NAVIGATOR_ORIENTATION_CHECK )
    {
        navigator_orientation_check_response( event, true );
    }
    else if( code == NAVIGATOR_ORIENTATION )
    {
        int angle = navigator_event_get_orientation_angle( event );

        //Let bbutil rotate current screen surface to this angle
        if( EXIT_FAILURE == bbutil_rotate_screen_surface( angle, g_BBMain_EGLSwapInterval ) )
        {
            fprintf( stderr, "Unable to handle orientation change\n" );
            return; // todo: handle errors... not likely to happen :)
        }

        EGLint surface_width, surface_height;

        eglQuerySurface( egl_disp, egl_surf, EGL_WIDTH, &surface_width );
        eglQuerySurface( egl_disp, egl_surf, EGL_HEIGHT, &surface_height );

        g_pGameCore->OnSurfaceChanged( 0, 0, surface_width, surface_height );
    }
    else if( code == NAVIGATOR_BACK )
    {
        g_pGameCore->OnButtons( GCBA_Down, GCBI_Back );
    }
    else if( code == NAVIGATOR_WINDOW_ACTIVE )
    {
        ClearKeysHeld();
        g_pGameCore->OnFocusGained();
        //g_pGameCore->OnSurfaceCreated();
    }
    else if( code == NAVIGATOR_WINDOW_INACTIVE )
    {
        ClearKeysHeld();
        g_pGameCore->OnFocusLost();
        //g_pGameCore->OnSurfaceLost();
    }
    //else if( code == NAVIGATOR_WINDOW_STATE )
    //{
    //  navigator_window_state_t state = navigator_event_get_window_state( event );

    //  if( state == NAVIGATOR_WINDOW_FULLSCREEN )
    //  {
    //      g_pGameCore->OnFocusGained();
    //  }
    //  else if( state == NAVIGATOR_WINDOW_THUMBNAIL )
    //  {
    //      g_pGameCore->OnFocusLost();
    //  }
    //  else if( state == NAVIGATOR_WINDOW_INVISIBLE )
    //  {
    //      g_pGameCore->OnFocusLost();
    //  }
    //}
}

void handleScreenEvent(bps_event_t *event)
{
    screen_event_t screen_event = screen_event_get_event( event );

    int screen_val;
    int pair[2];

    mtouch_event_t mtouch_event;
    int rc;
    int id;

    screen_get_event_property_iv( screen_event, SCREEN_PROPERTY_TYPE, &screen_val );
    screen_get_event_property_iv( screen_event, SCREEN_PROPERTY_SOURCE_POSITION, pair );

    switch( screen_val )
    {
    case SCREEN_EVENT_MTOUCH_TOUCH:
        rc = screen_get_mtouch_event( screen_event, &mtouch_event, 0 );
        if( rc ) // failed to get mtouch event
            return;

        id = mtouch_event.contact_id;

        g_pGameCore->OnTouch( GCBA_Down, id, (float)pair[0], (float)pair[1], 0, 0 ); // new press
        break;

    case SCREEN_EVENT_MTOUCH_MOVE:
        rc = screen_get_mtouch_event( screen_event, &mtouch_event, 0 );
        if( rc ) // failed to get mtouch event
            return;

        id = mtouch_event.contact_id;

        g_pGameCore->OnTouch( GCBA_Held, id, (float)pair[0], (float)pair[1], 0, 0 ); // new release
        break;

    case SCREEN_EVENT_MTOUCH_RELEASE:
        rc = screen_get_mtouch_event( screen_event, &mtouch_event, 0 );
        if( rc ) // failed to get mtouch event
            return;

        id = mtouch_event.contact_id;

        g_pGameCore->OnTouch( GCBA_Up, id, (float)pair[0], (float)pair[1], 0, 0 ); // still pressed
        break;

    case SCREEN_EVENT_POINTER:
        //This is a mouse move event, it is applicable to a device with a
        //USB mouse or simulator
        int buttons;

        screen_get_event_property_iv( screen_event, SCREEN_PROPERTY_BUTTONS, &buttons );

        if( buttons == SCREEN_LEFT_MOUSE_BUTTON )
        {
            if( h_mouseisdown == false )
            {
                g_pGameCore->OnTouch( GCBA_Down, 0, (float)pair[0], (float)pair[1], 0, 0 ); // new press
            }
            else
            {
                g_pGameCore->OnTouch( GCBA_Held, 0, (float)pair[0], (float)pair[1], 0, 0 ); // still pressed
            }

            h_mouseisdown = true;
        }
        else
        {
            if( h_mouseisdown )
            {
                g_pGameCore->OnTouch( GCBA_Up, 0, (float)pair[0], (float)pair[1], 0, 0 ); // new release

                h_mouseisdown = false;
            }
        }
        break;

    case SCREEN_EVENT_KEYBOARD:
        {
            int keyflags;
            screen_get_event_property_iv( screen_event, SCREEN_PROPERTY_KEY_FLAGS, &keyflags );

            // we ignore BB10 KEY_REPEAT messages and deal with it ourselves.
            if( (keyflags & KEY_SYM_VALID) )
            {
                int keycode;
                screen_get_event_property_iv( screen_event, SCREEN_PROPERTY_KEY_SYM, &keycode );

                if( (keyflags & KEY_DOWN) )
                {
                    if( !(keyflags & KEY_REPEAT) )
                    {
                        if( keycode < MAX_KEYS_WE_HANDLE_HELD_EVENTS_FOR )
                            h_keysheld[keycode] = true;

                        BBFUNC_HandleKeyboardEvents( GCBA_Down, keycode );
                    }
                }
                else // not down is up?!? works, but will it ever cause issues.
                {
                    if( keycode < MAX_KEYS_WE_HANDLE_HELD_EVENTS_FOR )
                        h_keysheld[keycode] = false;

                    BBFUNC_HandleKeyboardEvents( GCBA_Up, keycode );
                }
            }
        }
        break;
    }
}

int bbmain(const char* UUID)
{
    MyAssert( g_pGameCore );

    int rc;
    int exit_application = 0;

    // Create a screen context that will be used to create an EGL surface to to receive libscreen events
    screen_create_context( &screen_cxt, 0 );

    // Initialize BPS library
    bps_initialize();

    // Use utility code to initialize EGL for rendering with GL ES 2.0
    if( EXIT_SUCCESS != bbutil_init_egl( screen_cxt, g_BBMain_EGLSwapInterval ) )
    {
        fprintf( stderr, "bbutil_init_egl failed\n" );
        bbutil_terminate();
        screen_destroy_context( screen_cxt );
        return 0;
    }

#if MYFW_BLACKBERRY10
    if( UUID != 0 )
        g_pBBM = new BlackBerryMessenger( UUID );
#endif

    //Signal BPS library that navigator and screen events will be requested
    if( BPS_SUCCESS != screen_request_events( screen_cxt ) )
    {
        fprintf( stderr, "screen_request_events failed\n" );
        bbutil_terminate();
        screen_destroy_context( screen_cxt );
        bps_shutdown();
        return 0;
    }

    if( BPS_SUCCESS != navigator_request_events( 0 ) )
    {
        fprintf( stderr, "navigator_request_events failed\n" );
        bbutil_terminate();
        screen_destroy_context( screen_cxt );
        bps_shutdown();
        return 0;
    }

    //Signal BPS library that navigator orientation is not to be locked
    if( BPS_SUCCESS != navigator_rotation_lock( false ) )
    {
        fprintf( stderr, "navigator_rotation_lock failed\n" );
        bbutil_terminate();
        screen_destroy_context( screen_cxt );
        bps_shutdown();
        return 0;
    }

    EGLint surface_width, surface_height;

    eglQuerySurface( egl_disp, egl_surf, EGL_WIDTH, &surface_width );
    eglQuerySurface( egl_disp, egl_surf, EGL_HEIGHT, &surface_height );

    g_pGameCore->OnSurfaceCreated();
    g_pGameCore->OnSurfaceChanged( 0, 0, surface_width, surface_height );
    g_pGameCore->OneTimeInit();

    double lasttime = MyTime_GetSystemTime();

    g_pGameCore->m_pMediaPlayer->StartUpMediaPlayer(); // has to come after g_pGameCore->OneTimeInit();

#if MYFW_BLACKBERRY10
    g_pIAPManager = MyNew IAPManager();
#endif

    ClearKeysHeld();

    while( !exit_application )
    {
        double currtime = MyTime_GetSystemTime();
        double timepassed = currtime - lasttime;
        lasttime = currtime;

        //Request and process all available BPS events
        bps_event_t* event = NULL;

        while( true )
        {
            int timeout = 0;
            if( g_pGameCore->m_HasFocus == false || g_pGameCore->m_Settled == true )
                timeout = -1;

            StoreOldKeyHeldStateAndClearCurrent(); // part of hack to get keyheld messages.

#if USE_SCORELOOP
            g_pGameServiceManager->m_pScoreLoop->HandleEvents();
#endif

            //// deal with the payment manager.
            //if( g_pIAPManager->Tick() )
            //    g_pGameCore->m_Settled = false;

            rc = bps_get_event( &event, timeout );
            MyAssert( rc == BPS_SUCCESS );

            if( event )
            {
                g_pGameCore->m_Settled = false;

                int domain = bps_event_get_domain( event );

                if( domain == screen_get_domain() )
                {
                    handleScreenEvent( event );
                }
#if MYFW_BLACKBERRY10
                else if( domain == bbmsp_get_domain() )
                {
                    if( g_pBBM )
                        g_pBBM->HandleEvent( event );
                }
                else if( domain == paymentservice_get_domain() )
                {
                    if( g_pIAPManager )
                        g_pIAPManager->HandleEvent( event );
                }
#endif
                else if( domain == navigator_get_domain() )
                {
                    if( bps_event_get_code( event ) == NAVIGATOR_EXIT )
                    {
                        exit_application = 1;
                        g_pGameCore->OnPrepareToDie();
                        break;
                    }
                    else
                    {
                        handleNavigatorEvent( event );
                    }
                }
            }
            else
            {
                break;
            }
        }

        // send a key held message for each key held since previous frame.
        GenerateKeyHeldMessages();

        if( g_pGameCore->m_HasFocus )
        {
            //LOGInfo( LOGTag, "timepassed - %f\n", (float)timepassed );
            g_UnpausedTime += g_pGameCore->Tick( timepassed );
            g_pGameCore->OnDrawFrame();
            g_pGameCore->OnDrawFrameDone();
        }

        bbutil_swap();
        //LOGInfo( LOGTag, "swap\n" );
    }

    g_pGameCore->m_pMediaPlayer->ShutdownMediaPlayer();

    //Stop requesting events from libscreen
    screen_stop_events( screen_cxt );

    //Shut down BPS library for this process
    bps_shutdown();

#if USE_SCORELOOP
    g_pGameServiceManager->m_pScoreLoop->Shutdown();
#endif

    //Use utility code to terminate EGL setup
    bbutil_terminate();

#if MYFW_BLACKBERRY10
    SAFE_DELETE( g_pBBM );
#endif

    //Destroy libscreen context
    screen_destroy_context( screen_cxt );
    return 0;
}
