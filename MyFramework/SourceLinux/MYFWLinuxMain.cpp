//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// Adapted from: https://www.opengl.org/discussion_boards/showthread.php/177999-GCC-OpenGL-without-glut?p=1239444&viewfull=1#post1239444
// and https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)

#include "CommonHeader.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

bool g_EscapeButtonWillQuit;
bool g_CloseProgramRequested;

struct MyWin
{
    Display*    pDisplay;
    Window      win;
    bool        displayed;
    int         width;
    int         height;
    GLXContext  context;
    Colormap    colormap;
};

MyWin g_Window;

const int WIN_XPOS = 0;
const int WIN_YPOS = 0;

int g_InitialWidth;
int g_InitialHeight;

bool g_KeyStates[256];

int g_MouseX;
int g_MouseY;
bool g_MouseButtonStates[3];

static bool g_GameWantsLockedMouse = false;
static bool g_SystemMouseIsLocked = false;

static int g_MouseXPositionWhenLocked = 300;
static int g_MouseYPositionWhenLocked = 300;

bool MYFW_GetKey(int value)
{
    MyAssert( value >= 0 && value < 256 );
    return g_KeyStates[value];
}

void HandleKeyboardEvents(KeySym sym, unsigned char key, int x, int y, bool pressed)
{
    switch( tolower( key ) )
    {
    case 0:
        switch( sym )
        {
        case XK_Left:    key = MYKEYCODE_LEFT;    break;
        case XK_Up:      key = MYKEYCODE_UP;      break;
        case XK_Right:   key = MYKEYCODE_RIGHT;   break;
        case XK_Down:    key = MYKEYCODE_DOWN;    break;
        }
        break;
    }

    // if( pressed )
    //     LOGInfo( LOGTag, "Key down: %d\n", key );
    // else
    //     LOGInfo( LOGTag, "Key up: %d\n", key );
    
    g_KeyStates[key] = pressed;
}

void GenerateKeyboardEvents(GameCore* pGameCore)
{
    static unsigned int keys[256];
    static unsigned int keysold[256];

    for( int i=0; i<256; i++ )
    {
        keysold[i] = keys[i];
        keys[i] = MYFW_GetKey( i );

        if( keys[i] == 1 && keysold[i] == 0 )
        {
            // If the game is set to quit on escape, then quit.
            if( i == MYKEYCODE_ESC )
            {
                if( g_SystemMouseIsLocked == true )
                {
                    g_SystemMouseIsLocked = false;
                    //ShowCursor( true ); // TODO: Hide cursor
                }
                else
                {
                    if( g_EscapeButtonWillQuit )
                        g_CloseProgramRequested = true;
                }
            }
            
            pGameCore->OnKeyDown( i, i );
        }

        if( keys[i] == 0 && keysold[i] == 1 )
        {
            pGameCore->OnKeyUp( i, i );
        }
    }

    // if( keys[MYKEYCODE_LCTRL] && keys['M'] == 1 && keysold['M'] == 0 ) // new press
    //     h_moviemode = !h_moviemode;
    // if( keys[MYKEYCODE_LCTRL] && keys['S'] == 1 && keysold['S'] == 0 ) // new press
    //     h_takescreenshot = true;

    // if( keys[MYKEYCODE_LCTRL] && keys['I'] == 1 && keysold['I'] == 0 ) // new press
    // {
    //     if( g_pShaderManager )
    //         g_pShaderManager->InvalidateAllShaders( true );
    //     if( g_pTextureManager )
    //         g_pTextureManager->InvalidateAllTextures( true );
    //     if( g_pBufferManager )
    //         g_pBufferManager->InvalidateAllBuffers( true );
    // }

    if( keys['1'] == 1 && keysold['1'] == 0 )
    {
        XResizeWindow( g_Window.pDisplay, g_Window.win, g_InitialWidth, g_InitialHeight );
    }
    if( keys['2'] == 1 && keysold['2'] == 0 )
    {
        XResizeWindow( g_Window.pDisplay, g_Window.win, g_InitialWidth, (int)(g_InitialWidth*1.5f) );
    }
    if( keys['3'] == 1 && keysold['3'] == 0 )
    {
        XResizeWindow( g_Window.pDisplay, g_Window.win, g_InitialHeight, g_InitialHeight );
    }
    if( keys['4'] == 1 && keysold['4'] == 0 )
    {
        XResizeWindow( g_Window.pDisplay, g_Window.win, (int)(g_InitialWidth*1.5f), g_InitialWidth );
    }
}

void GenerateMouseEvents(GameCore* pGameCore)
{
    static unsigned int buttons[3];
    static unsigned int buttonsold[3];

    for( int i=0; i<3; i++ )
    {
        buttonsold[i] = buttons[i];
        buttons[i] = g_MouseButtonStates[i];
    }

    int mousex = g_MouseX;
    int mousey = g_MouseY;

    // buttons/fingers
    for( int i=0; i<3; i++ )
    {
        if( buttons[i] == 1 && buttonsold[i] == 0 )
        {
            if( g_GameWantsLockedMouse && g_SystemMouseIsLocked == false )
            {
                g_SystemMouseIsLocked = true;
                // ShowCursor( false );
                
                g_MouseXPositionWhenLocked = g_Window.width/2;
                g_MouseYPositionWhenLocked = g_Window.height/2;

                mousex = g_Window.width/2;
                mousey = g_Window.height/2;
            }

            pGameCore->OnTouch( GCBA_Down, i, (float)mousex, (float)mousey, 0, 0 ); // new press
        }

        if( buttons[i] == 0 && buttonsold[i] == 1 )
            pGameCore->OnTouch( GCBA_Up, i, (float)mousex, (float)mousey, 0, 0 ); // new release
    }

    int buttonstates = 0;
    for( int i=0; i<3; i++ )
    {
        if( buttons[i] == 1 && buttonsold[i] == 1 )
            buttonstates |= (1 << i);
    }

    // Game window wants mouse locked.
    if( g_GameWantsLockedMouse )
    {
        // Only send mouse movement messages (position diffs) if the system mouse is locked
        if( g_SystemMouseIsLocked )
        {
            // Set the mouse back to it's screen space position
            // POINT p;
            // p.x = g_MouseXPositionWhenLocked;
            // p.y = g_MouseYPositionWhenLocked;
            // ClientToScreen( hWnd, &p );
            // SetCursorPos( p.x, p.y );

            float xdiff = (float)mousex - g_MouseXPositionWhenLocked;
            float ydiff = (float)mousey - g_MouseYPositionWhenLocked;

            if( xdiff != 0 || ydiff != 0 )
            {
                pGameCore->OnTouch( GCBA_Held, buttonstates, xdiff, ydiff, 0, 0 );
            }
        }
        else
        {
            pGameCore->OnTouch( GCBA_Held, buttonstates, (float)mousex, (float)mousey, 0, 0 );
        }
    }
    else
    {
        // Only send mouse positions if system mouse isn't locked
        if( g_SystemMouseIsLocked == false )
        {
            pGameCore->OnTouch( GCBA_Held, buttonstates, (float)mousex, (float)mousey, 0, 0 );
        }
    }
}

void ResizeGLScene(int width, int height)
{
    if( height <= 0 ) height = 1;
    if( width <= 0 ) width = 1;

    g_Window.width = width;
    g_Window.height = height;
 
    if( g_pGameCore )
        g_pGameCore->OnSurfaceChanged( 0, 0, width, height );
}

// Try to find a framebuffer config that matches the specified pixel requirements.
GLXFBConfig chooseFBConfig(Display *display, int screen)
{
    // Get a matching framebuffer config.
    static const int Visual_attribs[] =
    {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };

    // FBConfigs were added in GLX version 1.3, check the version.
    int glx_major, glx_minor;
    if( glXQueryVersion( display, &glx_major, &glx_minor ) == false )
        return 0;
    
    if( ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )
        return 0;

    int fbcount;
    GLXFBConfig* pFBConfigs = glXChooseFBConfig( display, screen, Visual_attribs, &fbcount );
    if( pFBConfigs )
    {
        // Pick the FB config/visual with the most samples per pixel. Default to fbc 0.
        LOGInfo( LOGTag, "Getting XVisualInfos\n" );
        int best_fbc = 0, best_num_samp = -1;

        for( int i=0; i<fbcount; i++ )
        {
            XVisualInfo* vi = glXGetVisualFromFBConfig( display, pFBConfigs[i] );
            if( vi )
            {
                int samp_buf, samples;
                glXGetFBConfigAttrib( display, pFBConfigs[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                glXGetFBConfigAttrib( display, pFBConfigs[i], GLX_SAMPLES       , &samples  );

                LOGInfo( LOGTag, "  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d, SAMPLES = %d\n", 
                         i, vi -> visualid, samp_buf, samples );

                if( samp_buf && samples > best_num_samp )
                {
                    best_fbc = i;
                    best_num_samp = samples;
                }
            }

            XFree( vi );
        }

        GLXFBConfig bestFbc = pFBConfigs[best_fbc];
        
        XFree( pFBConfigs );

        return bestFbc;
    }

    return 0;
}
 
#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
static bool isExtensionSupported(const char *extList, const char *extension)
{
    const char* start;
    const char* where;
    const char* terminator;
    
    // Extension names should not have spaces.
    where = strchr( extension, ' ' );
    if( where || *extension == '\0' )
        return false;

    // It takes a bit of care to be fool-proof about parsing the
    //   OpenGL extensions string. Don't be fooled by sub-strings, etc.
    for( start=extList; ; )
    {
        where = strstr( start, extension );

        if( !where )
            break;

        terminator = where + strlen(extension);

        if( where == start || *(where - 1) == ' ' )
        {
            if( *terminator == ' ' || *terminator == '\0' )
                return true;
        }

        start = terminator;
    }

    return false;
}

GLXContext createContext(Display* pDisplay, int screen,
                         GLXFBConfig fbconfig, XVisualInfo* visinfo,
                         Window window)
{
    // Get the default screen's GLX extension list.
    const char* glxExts = glXQueryExtensionsString( pDisplay, DefaultScreen( pDisplay ) );

    // NOTE: It is not necessary to create or make current to a context before
    //       calling glXGetProcAddressARB.
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
            glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

    GLXContext ctx = 0;

    // Install an X error handler so the application won't exit if GL 3.0
    //   context allocation fails.
    //
    // Note this error handler is global.  All display connections in all threads
    //   of a process use the same error handler, so be sure to guard against other
    //   threads issuing X commands while this code is running.
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if( !isExtensionSupported( glxExts, "GLX_ARB_create_context" ) || !glXCreateContextAttribsARB )
    {
        LOGInfo( LOGTag, "glXCreateContextAttribsARB() not found ... using old-style GLX context\n" );
        ctx = glXCreateNewContext( pDisplay, fbconfig, GLX_RGBA_TYPE, 0, True );
    }
    else // If it does, try to get a GL 3.0 context!
    {
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };

        LOGInfo( LOGTag, "Creating context\n" );
        ctx = glXCreateContextAttribsARB( pDisplay, fbconfig, 0, True, context_attribs );

        // Sync to ensure any errors generated are processed.
        XSync( pDisplay, False );
        if( !ctxErrorOccurred && ctx )
        {
            LOGInfo( LOGTag, "Created GL 3.0 context\n" );
        }
        else
        {
            // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
            // When a context version below 3.0 is requested, implementations will
            //   return the newest context version compatible with OpenGL versions less
            //   than version 3.0.
            // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
            context_attribs[1] = 1;
            // GLX_CONTEXT_MINOR_VERSION_ARB = 0
            context_attribs[3] = 0;

            ctxErrorOccurred = false;

            LOGInfo( LOGTag, "Failed to create GL 3.0 context ... using old-style GLX context\n" );
            ctx = glXCreateContextAttribsARB( pDisplay, fbconfig, 0, True, context_attribs );
        }
    }

    // Sync to ensure any errors generated are processed.
    XSync( pDisplay, False );

    // Restore the original error handler
    XSetErrorHandler( oldHandler );

    if( ctxErrorOccurred || !ctx )
    {
        LOGError( LOGTag, "Failed to create an OpenGL context\n" );
        return 0;
    }

    // Verifying that context is a direct context
    if( glXIsDirect( pDisplay, ctx ) == false )
    {
        LOGInfo( LOGTag, "Indirect GLX rendering context obtained\n" );
    }
    else
    {
        LOGInfo( LOGTag, "Direct GLX rendering context obtained\n" );
    }

    LOGInfo( LOGTag, "Making context current\n" );
    glXMakeCurrent( pDisplay, g_Window.win, ctx );

    return ctx;
}

//----------------------------------------------------------------------------
 
bool createWindow()
{
    // Init X and GLX.
    g_Window.displayed = false;
    Display* pDisplay = g_Window.pDisplay = XOpenDisplay( ":0.0" );
    if( pDisplay == 0 )
    {
        LOGError( LOGTag, "Cannot open X display\n" );
        return false;
    }

    int    screen   = DefaultScreen( pDisplay );
    Window root_win = RootWindow( pDisplay, screen );

    if( !glXQueryExtension( pDisplay, 0, 0 ) )
    {
        LOGError( LOGTag, "X Server doesn't support GLX extension\n" );
        return false;
    }

    // Pick an FBconfig and visual.
    GLXFBConfig fbconfig = chooseFBConfig( pDisplay, screen );
    if( fbconfig == 0 )
    {
        LOGError( LOGTag, "Failed to get GLXFBConfig\n" );
        return false;
    }

    XVisualInfo* pVisInfo = glXGetVisualFromFBConfig( pDisplay, fbconfig );
    if( pVisInfo == 0 )
    {
        LOGError( LOGTag, "Failed to get XVisualInfo\n" );
        return false;
    }
    LOGInfo( LOGTag, "X Visual ID = 0x%.2x\n", int( pVisInfo->visualid ) );

    // Create the X window.
    XSetWindowAttributes winAttr;
    
    winAttr.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | 
                             PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
    winAttr.background_pixmap = None;
    winAttr.background_pixel  = 0;
    winAttr.border_pixel      = 0;

    g_Window.colormap = XCreateColormap( pDisplay, root_win, pVisInfo->visual, AllocNone );
    winAttr.colormap = g_Window.colormap;

    unsigned int mask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;

    Window win = g_Window.win = XCreateWindow( pDisplay, root_win,
                                               WIN_XPOS, WIN_YPOS,
                                               g_Window.width, g_Window.height, 0,
                                               pVisInfo->depth, InputOutput,
                                               pVisInfo->visual, mask, &winAttr );

    XStoreName( g_Window.pDisplay, win, "My GLX Window");

    // Create an OpenGL context and attach it to our X window.
    g_Window.context = createContext( pDisplay, screen, fbconfig, pVisInfo, win );

    // Display the window
    XMapWindow( pDisplay, win );

    if( !glXMakeCurrent( pDisplay, win, g_Window.context ) )
    {
        LOGError( LOGTag, "glXMakeCurrent failed.\n" );
        return false;
    }

    checkGlError( "createWindow()" );

    LOGInfo( LOGTag, "Window Size    = %d x %d\n", g_Window.width, g_Window.height );

    return true;
}

//----------------------------------------------------------------------------

void processXEvents( Atom wm_protocols, Atom wm_delete_window )
{
    while( XEventsQueued( g_Window.pDisplay, QueuedAfterFlush ) )
    {
        XEvent event;

        XNextEvent( g_Window.pDisplay, &event );

        if( event.xany.window != g_Window.win )
            continue;

        switch( event.type )
        {
        case MapNotify:
            {
                g_Window.displayed = true;
            }
            break;

        case ConfigureNotify:
            {
                XConfigureEvent &cevent = event.xconfigure;
                ResizeGLScene( cevent.width, cevent.height );
            }
            break;

        case KeyPress:
            {
                char      chr;
                KeySym    symbol;
                XComposeStatus status;

                XLookupString( &event.xkey, &chr, 1, &symbol, &status );

                HandleKeyboardEvents( symbol, chr, event.xkey.x, event.xkey.y, true );
            }
            break;

        case KeyRelease:
            {
                char      chr;
                KeySym    symbol;
                XComposeStatus status;

                XLookupString( &event.xkey, &chr, 1, &symbol, &status );

                HandleKeyboardEvents( symbol, chr, event.xkey.x, event.xkey.y, false );
            }
            break;

        case MotionNotify:
            {
                g_MouseX = event.xmotion.x;
                g_MouseY = event.xmotion.y;
            }
            break;

        case ButtonPress:
            g_MouseButtonStates[event.xbutton.button - 1] = true;
            //LOGInfo( LOGTag, "Button pressed  : %d\n", event.xbutton.button );
            break;

        case ButtonRelease:
            g_MouseButtonStates[event.xbutton.button - 1] = false;
            //LOGInfo( LOGTag, "Button released : %d\n", event.xbutton.button );
            break;            

        case ClientMessage:
            {
                if( event.xclient.message_type      == wm_protocols &&
                    Atom( event.xclient.data.l[0] ) == wm_delete_window )
                {
                    //LOGInfo( LOGTag, "Received WM_DELETE_WINDOW\n" );
                    exit( 0 );
                }
            }
            break;
        }
    }
}

//----------------------------------------------------------------------------

void mainLoop()
{
    // Register to receive window close events (the "X" window manager button)
    Atom wm_protocols     = XInternAtom( g_Window.pDisplay, "WM_PROTOCOLS"    , False);
    Atom wm_delete_window = XInternAtom( g_Window.pDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols( g_Window.pDisplay, g_Window.win, &wm_delete_window, True );

    double lasttime = MyTime_GetSystemTime();

    g_EscapeButtonWillQuit = true;
    g_CloseProgramRequested = false;

    while( true ) 
    {
        // Process OS events.
        processXEvents( wm_protocols, wm_delete_window );

        // Calculate elapsed time.
        double currtime = MyTime_GetSystemTime();
        double timepassed = currtime - lasttime;
        lasttime = currtime;
        
        // Redraw window (after it's mapped).
        if( g_Window.displayed )
        {
            GenerateKeyboardEvents( g_pGameCore );
            GenerateMouseEvents( g_pGameCore );
    
            // Tick and draw the game.
            g_pGameCore->OnDrawFrameStart( 0 );
            g_UnpausedTime += g_pGameCore->Tick( timepassed );
            g_pGameCore->OnDrawFrame( 0 );
            g_pGameCore->OnDrawFrameDone();

            glXSwapBuffers( g_Window.pDisplay, g_Window.win );

            checkGlError( "glXSwapBuffers" );
        }

        if( g_CloseProgramRequested )
            break;
    }
}

//----------------------------------------------------------------------------

int MYFWLinuxMain(int width, int height)
{
    // Init globals.
    g_Window.width = width, g_Window.height = height;

    g_InitialWidth = width;
    g_InitialHeight = height;

    // Create context and window.
    if( createWindow() == false )
    {
        exit( 0 );
    }

    // Locate GL functions.
    OpenGL_InitExtensions();

    // Create and initialize our Game object.
    g_pGameCore->OnSurfaceCreated();
    g_pGameCore->OnSurfaceChanged( 0, 0, width, height );
    g_pGameCore->OneTimeInit();

    // Go.
    mainLoop();

    // Shutdown.
    delete( g_pGameCore );
    g_pGameCore = 0;

    glXMakeCurrent( g_Window.pDisplay, 0, 0 );
    glXDestroyContext( g_Window.pDisplay, g_Window.context );

    XDestroyWindow( g_Window.pDisplay, g_Window.win );
    XFreeColormap( g_Window.pDisplay, g_Window.colormap );
    XCloseDisplay( g_Window.pDisplay );

    return 0;
}
