//
// Copyright (c) 2012-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "GLExtensions.h"
#include "MYFWWinMain.h"
#include "Screenshot.h"
#include "WGLExtensions.h"
#include "../SourceCommon/Core/GameCore.h"
#include "../SourceCommon/Helpers/ExternalTools.h"
#include "../SourceCommon/Helpers/MyTime.h"
#include "../SourceCommon/JSON/cJSONHelpers.h"
#include "../SourceCommon/Meshes/BufferManager.h"
#include "../SourceCommon/Renderers/BaseClasses/Renderer_Enums.h"
#include "../SourceCommon/Shaders/ShaderManager.h"
#include "../SourceCommon/Textures/TextureManager.h"

// Initialize opengl window on windows, huge chunks taken from nehe.
//    http://nehe.gamedev.net/tutorial/creating_an_opengl_window_%28win32%29/13001/
// Update to GL 3+ using this info:
//    https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/

#if MYFW_USING_IMGUI
unsigned int g_GLCanvasIDActive = 0;
#endif

#define SHOW_SYSTEM_MOUSE_DEBUG_LOG             0
#define SHOW_SYSTEM_MOUSE_DEBUG_LOG_MOVEMENT    0

HWND g_hWnd = 0;

static bool g_EscapeButtonWillQuit;
static bool g_CloseProgramRequested;

static int g_RequestedWidth;
static int g_RequestedHeight;

static int g_InitialWidth;
static int g_InitialHeight;

static HGLRC g_hRenderingContext = 0;
static HDC g_hDeviceContext = 0;
static HINSTANCE g_hInstance = 0;

static int g_WindowWidth = 0;
static int g_WindowHeight = 0;
static bool g_KeyStates[256];
static bool g_MouseButtonStates[3];
static int g_MouseWheelDelta = 0;
static Vector2 g_RawMouseDelta(0);
static bool g_WindowIsVisible = true;
static bool g_OnlyUpdateOnEvents = false; // TODO: Change this setting depending on if gameplay or editor is active.
static bool g_FullscreenMode = true;

static bool g_RawMouseInputInitialized = false;
static bool g_SystemMouseIsLocked = false;
static POINT g_MousePositionBeforeLock;

static int g_MouseXPositionWhenLocked = -1;
static int g_MouseYPositionWhenLocked = -1;

static bool h_moviemode = false;
static bool h_takescreenshot = false;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void KillGLWindow(bool destroyInstance);
bool FailAndCleanup(const char* pMessage);
bool CreateTempGLWindow();

bool MYFW_GetKey(int value)
{
    MyAssert( value >= 0 && value < 256 );
    return g_KeyStates[value];
}

GLvoid ResizeGLScene(GLsizei width, GLsizei height)
{
    if( height <= 0 ) height = 1;
    if( width <= 0 ) width = 1;

    g_WindowWidth = width;
    g_WindowHeight = height;
 
    if( g_pGameCore )
        g_pGameCore->OnSurfaceChanged( 0, 0, width, height );
}

void SetWindowSize(int width, int height)
{
    int maxwidth = GetSystemMetrics( SM_CXFULLSCREEN );
    int maxheight = GetSystemMetrics( SM_CYFULLSCREEN );

    float aspect = (float)width / height;

    if( width > maxwidth )
    {
        width = maxwidth;
        height = (int)(maxwidth / aspect);
    }

    if( height > maxheight )
    {
        width = (int)(maxheight * aspect);
        height = maxheight;
    }

    // Typecast from LONG_PTR to DWORD should be ok when querying GWL_STYLE and GWL_EXSTYLE.
    DWORD dwStyle = (DWORD)GetWindowLongPtr( g_hWnd, GWL_STYLE );
    DWORD dwExStyle = (DWORD)GetWindowLongPtr( g_hWnd, GWL_EXSTYLE );
    HMENU menu = GetMenu( g_hWnd );

    // Calculate the full size of the window needed to match our client area of width/height.
    RECT WindowRect = { 0, 0, width, height };
    AdjustWindowRectEx( &WindowRect, dwStyle, menu ? TRUE : FALSE, dwExStyle );

    int windowwidth = WindowRect.right - WindowRect.left;
    int windowheight = WindowRect.bottom - WindowRect.top;
    
    SetWindowPos( g_hWnd, 0, 0, 0, windowwidth, windowheight, SWP_NOZORDER | SWP_NOMOVE );
    
    ResizeGLScene( width, height );
}

void GenerateKeyboardEvents(GameCore* pGameCore)
{
    static unsigned int keys[256];
    static unsigned int keysOld[256];

    for( int i=0; i<256; i++ )
    {
        keysOld[i] = keys[i];
        keys[i] = MYFW_GetKey( i );

        if( keys[i] == 1 && keysOld[i] == 0 )
        {
            // If the game is set to quit on escape, then quit.
            if( i == MYKEYCODE_ESC )
            {
                if( g_SystemMouseIsLocked )
                {
#if !MYFW_USING_WX
                    UnlockSystemMouse();
#endif
                }
                else
                {
                    if( g_EscapeButtonWillQuit )
                    {
                        g_pGameCore->RequestClose();
                        //g_CloseProgramRequested = true;
                    }
                }
            }
            
            pGameCore->OnKeyDown( i, i );

            //LOGInfo( LOGTag, "Calling pGameCore->OnKeyDown( %d, %d )\n", i, i );
        }

        if( keys[i] == 0 && keysOld[i] == 1 )
        {
            pGameCore->OnKeyUp( i, i );
        }
    }

#if !MYFW_EDITOR
    if( keys[MYKEYCODE_LCTRL] && keys['M'] == 1 && keysOld['M'] == 0 ) // new press
        h_moviemode = !h_moviemode;
    if( keys[MYKEYCODE_LCTRL] && keys['S'] == 1 && keysOld['S'] == 0 ) // new press
        h_takescreenshot = true;

    if( keys[MYKEYCODE_LCTRL] && keys['I'] == 1 && keysOld['I'] == 0 ) // new press
    {
        ShaderManager* pShaderManager = pGameCore->GetManagers()->GetShaderManager();
        TextureManager* pTextureManager = pGameCore->GetManagers()->GetTextureManager();
        BufferManager* pBufferManager = pGameCore->GetManagers()->GetBufferManager();

        if( pShaderManager )
            pShaderManager->InvalidateAllShaders( true );
        if( pTextureManager )
            pTextureManager->InvalidateAllTextures( true );
        if( pBufferManager )
            pBufferManager->InvalidateAllBuffers( true );
    }

    if( keys['1'] == 1 && keysOld['1'] == 0 )
    {
        SetWindowSize( g_RequestedWidth, g_RequestedHeight );
    }
    if( keys['2'] == 1 && keysOld['2'] == 0 )
    {
        SetWindowSize( g_InitialWidth, (int)(g_InitialWidth*1.5f) );
    }
    if( keys['3'] == 1 && keysOld['3'] == 0 )
    {
        SetWindowSize( g_InitialHeight, g_InitialHeight );
    }
    if( keys['4'] == 1 && keysOld['4'] == 0 )
    {
        SetWindowSize( (int)(g_InitialWidth*1.5f), g_InitialWidth );
    }
#endif
}

void GetMouseCoordinates(int* mx, int* my)
{
    POINT p;
    if( GetCursorPos( &p ) )
    {
        if( ScreenToClient( g_hWnd, &p ) )
        {
            *mx = p.x;
            *my = p.y;
        }
    }
}

// Will return whether or not this call locked the mouse.
bool LockSystemMouse()
{
    if( g_SystemMouseIsLocked == false )
    {
        // Store the old cursor position.
        GetCursorPos( &g_MousePositionBeforeLock );

        if( SHOW_SYSTEM_MOUSE_DEBUG_LOG )
            LOGInfo( "SystemMouse", "System Mouse Locked: (%d, %d);\n", g_MousePositionBeforeLock.x, g_MousePositionBeforeLock.y );

        g_SystemMouseIsLocked = true;
        ShowCursor( false );
        
        // Lock the mouse to the center of the screen.
        g_MouseXPositionWhenLocked = g_WindowWidth / 2;
        g_MouseYPositionWhenLocked = g_WindowHeight / 2;

        // Set the mouse to it's screen space position.
        POINT lockedMouseScreenPos;
        lockedMouseScreenPos.x = g_MouseXPositionWhenLocked;
        lockedMouseScreenPos.y = g_MouseYPositionWhenLocked;
        ClientToScreen( g_hWnd, &lockedMouseScreenPos );
        SetCursorPos( lockedMouseScreenPos.x, lockedMouseScreenPos.y );

        return true;
    }

    return false;
}

// Will return whether or not this call unlocked the mouse.
bool UnlockSystemMouse()
{
    if( g_SystemMouseIsLocked == true )
    {
        if( SHOW_SYSTEM_MOUSE_DEBUG_LOG )
            LOGInfo( "SystemMouse", "System Mouse Unlocked - (%d, %d);\n", g_MousePositionBeforeLock.x, g_MousePositionBeforeLock.y );

        g_SystemMouseIsLocked = false;
        ShowCursor( true );
        
        // Restore the position of the cursor before it was locked.
        SetCursorPos( g_MousePositionBeforeLock.x, g_MousePositionBeforeLock.y );

        return true;
    }

    return false;
}

void SetMouseLock(bool lock)
{
    if( lock == true )
    {
        LockSystemMouse();
    }
    
    if( lock == false )
    {
        UnlockSystemMouse();
    }
}

bool IsMouseLocked()
{
    return g_SystemMouseIsLocked;
}

void GenerateMouseEvents(GameCore* pGameCore)
{
    static unsigned int buttons[3];
    static unsigned int buttonsOld[3];

    for( int i=0; i<3; i++ )
    {
        buttonsOld[i] = buttons[i];
        buttons[i] = g_MouseButtonStates[i];

        if( buttons[i] != 0 )
            int bp = 1;
    }

    int mousex;
    int mousey;
    GetMouseCoordinates( &mousex, &mousey );

    // Buttons/fingers.
    for( int i=0; i<3; i++ )
    {
        if( buttons[i] == 1 && buttonsOld[i] == 0 )
        {
            if( SHOW_SYSTEM_MOUSE_DEBUG_LOG )
                LOGInfo( "SystemMouse", "Mouse down (%d)\n", i );
            pGameCore->OnTouch( GCBA_Down, i, (float)mousex, (float)mousey, 0, 0 ); // New press.
        }

        if( buttons[i] == 0 && buttonsOld[i] == 1 )
        {
            if( SHOW_SYSTEM_MOUSE_DEBUG_LOG )
                LOGInfo( "SystemMouse", "Mouse up (%d)\n", i );
            pGameCore->OnTouch( GCBA_Up, i, (float)mousex, (float)mousey, 0, 0 ); // New release.
        }
    }

    int buttonStates = 0;
    for( int i=0; i<3; i++ )
    {
        if( buttons[i] == 1 && buttonsOld[i] == 1 )
            buttonStates |= (1 << i);
    }

    if( g_MouseWheelDelta != 0 )
    {
        pGameCore->OnTouch( GCBA_Wheel, buttonStates, (float)mousex, (float)mousey, (float)g_MouseWheelDelta/WHEEL_DELTA, 0 );
        g_MouseWheelDelta = 0;
    }

    // Only send relative mouse movement messages if the system mouse is locked.
    if( g_SystemMouseIsLocked )
    {
        if( g_RawMouseInputInitialized )
        {
            if( SHOW_SYSTEM_MOUSE_DEBUG_LOG && SHOW_SYSTEM_MOUSE_DEBUG_LOG_MOVEMENT )
                LOGInfo( "SystemMouse", "Raw mouse move relative (%0.2f, %0.2f)\n", g_RawMouseDelta.x, g_RawMouseDelta.y );

            if( g_RawMouseDelta.x != 0 || g_RawMouseDelta.y != 0 )
            {
                if( buttonStates == 0 )
                    pGameCore->OnTouch( GCBA_RelativeMovement, -1, g_RawMouseDelta.x, g_RawMouseDelta.y, 0, 0 );
                if( buttonStates & 1 << 0 )
                    pGameCore->OnTouch( GCBA_RelativeMovement, 0, g_RawMouseDelta.x, g_RawMouseDelta.y, 0, 0 );
                if( buttonStates & 1 << 1 )
                    pGameCore->OnTouch( GCBA_RelativeMovement, 1, g_RawMouseDelta.x, g_RawMouseDelta.y, 0, 0 );
                if( buttonStates & 1 << 2 )
                    pGameCore->OnTouch( GCBA_RelativeMovement, 2, g_RawMouseDelta.x, g_RawMouseDelta.y, 0, 0 );
            }

            // Set the mouse back to it's screen space position.
            POINT lockedMouseScreenPos;
            lockedMouseScreenPos.x = g_MouseXPositionWhenLocked;
            lockedMouseScreenPos.y = g_MouseYPositionWhenLocked;            
            ClientToScreen( g_hWnd, &lockedMouseScreenPos );
            SetCursorPos( lockedMouseScreenPos.x, lockedMouseScreenPos.y );
        }
        else
        {
            // TODO: There are issues with this if the global display scale is set to anything but 1.
            //       GetCursor can return a different position than SetCursor on "sub-pixels" causing drift.

            // Set the mouse back to it's screen space position.
            POINT lockedMouseScreenPos;
            lockedMouseScreenPos.x = g_MouseXPositionWhenLocked;
            lockedMouseScreenPos.y = g_MouseYPositionWhenLocked;            
            ClientToScreen( g_hWnd, &lockedMouseScreenPos );
            SetCursorPos( lockedMouseScreenPos.x, lockedMouseScreenPos.y );

            POINT currentMouseScreenPos;
            currentMouseScreenPos.x = mousex;
            currentMouseScreenPos.y = mousey;            
            ClientToScreen( g_hWnd, &currentMouseScreenPos );

            int xdiff = currentMouseScreenPos.x - lockedMouseScreenPos.x;
            int ydiff = currentMouseScreenPos.y - lockedMouseScreenPos.y;

            if( SHOW_SYSTEM_MOUSE_DEBUG_LOG && SHOW_SYSTEM_MOUSE_DEBUG_LOG_MOVEMENT )
                LOGInfo( "SystemMouse", "Mouse move relative (%d, %d)\n", xdiff, ydiff );

            if( xdiff != 0 || ydiff != 0 )
            {
                if( buttonStates == 0 )
                    pGameCore->OnTouch( GCBA_RelativeMovement, -1, (float)xdiff, (float)ydiff, 0, 0 );
                if( buttonStates & 1 << 0 )
                    pGameCore->OnTouch( GCBA_RelativeMovement, 0, (float)xdiff, (float)ydiff, 0, 0 );
                if( buttonStates & 1 << 1 )
                    pGameCore->OnTouch( GCBA_RelativeMovement, 1, (float)xdiff, (float)ydiff, 0, 0 );
                if( buttonStates & 1 << 2 )
                    pGameCore->OnTouch( GCBA_RelativeMovement, 2, (float)xdiff, (float)ydiff, 0, 0 );
            }
        }
    }
    else
    {
        // If the system mouse isn't locked, send absolute mouse positions.
        // If the game had requested a mouse lock, it should ignore these.
        // These messages are needed for imgui windows over a game that wants the mouse locked.
        if( SHOW_SYSTEM_MOUSE_DEBUG_LOG && SHOW_SYSTEM_MOUSE_DEBUG_LOG_MOVEMENT )
            LOGInfo( "SystemMouse", "Mouse move absolute (%d, %d)\n", mousex, mousey );

        if( buttonStates == 0 )
            pGameCore->OnTouch( GCBA_Held, -1, (float)mousex, (float)mousey, 0, 0 );
        if( buttonStates & 1 << 0 )
            pGameCore->OnTouch( GCBA_Held, 0, (float)mousex, (float)mousey, 0, 0 );
        if( buttonStates & 1 << 1 )
            pGameCore->OnTouch( GCBA_Held, 1, (float)mousex, (float)mousey, 0, 0 );
        if( buttonStates & 1 << 2 )
            pGameCore->OnTouch( GCBA_Held, 2, (float)mousex, (float)mousey, 0, 0 );
    }

    g_RawMouseDelta.Set( 0, 0 );
}

bool CreateTempGLWindow()
{
    // Create a temp window to setup GL extensions:
    
    // Create the window.
    g_hWnd = CreateWindow( "OpenGL", "Fake Window",           // Window class, Title.
                           WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // Style.
                           0, 0,                              // Position.
                           1, 1,                              // Size.
                           nullptr, nullptr,                  // Parent window, Menu.
                           g_hInstance, nullptr );            // Instance, Param.
    if( g_hWnd == nullptr )
        return FailAndCleanup( "Failed to create temp window." );

    // Get the DC.
    g_hDeviceContext = GetDC( g_hWnd );
    if( g_hDeviceContext == nullptr )
        return FailAndCleanup( "Failed to get the temp device context." );

    // Choose a temp PixelFormat.
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory( &pfd, sizeof(pfd) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;

    GLuint pixelFormat = ChoosePixelFormat( g_hDeviceContext, &pfd );
    if( pixelFormat == 0 )
        return FailAndCleanup( "Failed to find a suitable PixelFormat." );

    // Set the pixel format of the device context.
    bool result = SetPixelFormat( g_hDeviceContext, pixelFormat, &pfd );
    if( result == 0 )
        return FailAndCleanup( "Failed to set the PixelFormat." );

    // Create GL rendering context.
    g_hRenderingContext = wglCreateContext( g_hDeviceContext );
    if( g_hRenderingContext == nullptr )
        return FailAndCleanup( "Failed to create a GL rendering context." );

    // Activate.
    if( wglMakeCurrent( g_hDeviceContext, g_hRenderingContext ) == false )
        return FailAndCleanup( "Failed to activate the GL rendering context." );

    return true; // Everything worked out.
}

bool CreateGLWindow(char* title, int width, int height, unsigned char colorBits, unsigned char zBits, unsigned char stencilBits, bool fullScreenFlag)
{
    DWORD dwExStyle;
    DWORD dwStyle;

    RECT WindowRect;
    WindowRect.left = (long)0;
    WindowRect.right = (long)width;
    WindowRect.top = (long)0;
    WindowRect.bottom = (long)height;

    g_FullscreenMode = fullScreenFlag;

    g_hInstance = GetModuleHandle( nullptr ); // Grab an instance for our application.

    // Define and register the window class.
    {
        WNDCLASSEX wc;
        ZeroMemory( &wc, sizeof(wc) );
        wc.cbSize = sizeof( wc );
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Redraw on move, and own DC for window.
        wc.lpfnWndProc = (WNDPROC)WndProc;              // WndProc handles messages.
        wc.cbClsExtra = 0;                              // No extra window data.
        wc.cbWndExtra = 0;                              // No extra window data.
        wc.hInstance = g_hInstance;                     // Set the instance.
        wc.hIcon = LoadIcon( 0, IDI_WINLOGO );          // Load the default icon.
        wc.hCursor = LoadCursor( 0, IDC_ARROW );        // Load the arrow pointer.
        wc.hbrBackground = 0;                           // No background required for GL.
        wc.lpszMenuName = nullptr;                      // We don't want a menu.
        wc.lpszClassName = "OpenGL";                    // Set the class name.

        // Attempt to register the Window Class.
        if( !RegisterClassEx( &wc ) )
        {
            MessageBox( 0, "Failed To Register The Window Class.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
            return false;
        }
    }

    // Create a temporary GL window and context to allow us to grab
    //     the WGL extension functions needed to generate proper GL context.
    {
        if( CreateTempGLWindow() == false )
            return false;

        // Initialize Windows OpenGL Extensions, must be done after OpenGL Context is created.
        WGL_InitContextCreationExtensions();

        // Destroy the temp GL window.
        KillGLWindow( false );
    }

    if( g_FullscreenMode )
    {
        DEVMODE dmScreenSettings;                                   // Device mode.
        memset( &dmScreenSettings, 0, sizeof( dmScreenSettings ) ); // Makes sure memory's cleared.
        dmScreenSettings.dmSize = sizeof( dmScreenSettings );       // Size of the DEVMODE structure.
        dmScreenSettings.dmPelsWidth  = width;                      // Selected screen width.
        dmScreenSettings.dmPelsHeight = height;                     // Selected screen height.
        dmScreenSettings.dmBitsPerPel = colorBits;                  // Selected bits per pixel.
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results. NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
        if( ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
        {
            // If The Mode Fails, Offer Two Options. Quit Or Run In A Window.
            if( MessageBox( 0, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "", MB_YESNO|MB_ICONEXCLAMATION ) == IDYES )
            {
                g_FullscreenMode = false;
            }
            else
            {
                MessageBox( 0, "Program will now Close.", "ERROR", MB_OK|MB_ICONSTOP );
                return false;
            }
        }
    }

    if( g_FullscreenMode )
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
        LockSystemMouse();
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;
    }

    AdjustWindowRectEx( &WindowRect, dwStyle, false, dwExStyle );   // Adjust window to true requested size.

    // Create our window.
    {
        g_hWnd = CreateWindowEx( dwExStyle,                            // Extended style for the window.
                                 "OpenGL",                             // Class name.
                                 title,                                // Window title.
                                 WS_CLIPSIBLINGS | WS_CLIPCHILDREN |   // Required window style.
                                   dwStyle,                            // Selected window style.
                                 0, 0,                                 // Window position.
                                 WindowRect.right-WindowRect.left,     // Calculate adjusted window width.
                                 WindowRect.bottom-WindowRect.top,     // Calculate adjusted window height.
                                 nullptr,                              // No parent window.
                                 nullptr,                              // No menu.
                                 g_hInstance,                          // Instance.
                                 nullptr );                            // Don't pass anything to WM_CREATE.
        if( g_hWnd == nullptr )
            return FailAndCleanup( "Failed to create a window." );
    }

    // Get the device context.
    {
        g_hDeviceContext = GetDC( g_hWnd );
        if( g_hDeviceContext == nullptr )
            return FailAndCleanup( "Failed to get the device context." );
    }

    // Choose a pixel format.
    {
        const int pixelAttribs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     colorBits,
            WGL_ALPHA_BITS_ARB,     0,
            WGL_DEPTH_BITS_ARB,     zBits,
            WGL_STENCIL_BITS_ARB,   stencilBits,
            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
            WGL_SAMPLES_ARB,        1,
            0
        };

        int pixelFormats[1];
        unsigned int numFormats;
        bool status = wglChoosePixelFormatARB( g_hDeviceContext, pixelAttribs, nullptr, 1, pixelFormats, &numFormats );
        if( status == false || numFormats == 0 )
            return FailAndCleanup( "Failed to find a suitable PixelFormat." );

        // Set the pixel format of the device context.
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat( g_hDeviceContext, pixelFormats[0], sizeof(pfd), &pfd );
        bool result = SetPixelFormat( g_hDeviceContext, pixelFormats[0], &pfd );
        if( result == 0 )
            return FailAndCleanup( "Failed to set the PixelFormat." );
    }

    // Create and activate a rendering context.
    {
        const int major_min = 4, minor_min = 5;
        int contextAttribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
            WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
            //WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            0
        };

        g_hRenderingContext = wglCreateContextAttribsARB( g_hDeviceContext, nullptr, contextAttribs );
        if( g_hRenderingContext == nullptr )
            return FailAndCleanup( "Failed to create a GL rendering context." );

        // Activate.
        if( wglMakeCurrent( g_hDeviceContext, g_hRenderingContext ) == false )
            return FailAndCleanup( "Failed to activate the GL rendering context." );
    }

    ShowWindow( g_hWnd, SW_SHOW );   // Show the window.
    SetForegroundWindow( g_hWnd );   // Slightly higher priority.
    SetFocus( g_hWnd );              // Sets keyboard focus to the window.
    ResizeGLScene( width, height );  // Set up our perspective GL screen.

#if MYFW_EDITOR
    DragAcceptFiles( g_hWnd, TRUE );
#endif

    return true;
}

bool FailAndCleanup(const char* pMessage)
{
    KillGLWindow( true );
    MessageBox( 0, pMessage, "ERROR", MB_OK|MB_ICONEXCLAMATION );
    return false;
}

void KillGLWindow(bool destroyInstance)
{
    if( g_FullscreenMode )
    {
        ChangeDisplaySettings( 0, 0 );
        UnlockSystemMouse();
    }

    if( g_hRenderingContext )
    {
        if( !wglMakeCurrent( 0, 0 ) )
        {
            MessageBox( 0, "Release of device context and rendering context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        }

        if( !wglDeleteContext( g_hRenderingContext ) )
        {
            MessageBox( 0, "Release rendering context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        }
    }

    if( g_hDeviceContext && !ReleaseDC( g_hWnd, g_hDeviceContext ) )
    {
        MessageBox( 0, "Release device context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
    }

    if( g_hWnd && !DestroyWindow( g_hWnd ) )
    {
        MessageBox( 0, "Could not release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
    }

    if( destroyInstance )
    {
        if( g_hInstance && !UnregisterClass( "OpenGL", g_hInstance ) )
        {
            MessageBox( 0, "Could not unregister class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        }
        g_hInstance = 0;
    }

    g_hRenderingContext = 0;
    g_hDeviceContext = 0;
    g_hWnd = 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
    case WM_ACTIVATE:
        {
            if( !HIWORD(wParam) )
            {
                g_WindowIsVisible = true;
            }
            else
            {
                g_WindowIsVisible = false;
            } 
        }
        return 0;

    case WM_SYSCOMMAND:
        {
            switch( wParam )
            {
                // Don't let screensaver or monitor power save mode kick in.
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;

                // Hotkeys are handled manually elsewhere.
                case SC_KEYMENU:
                    // Let common alt keys be handled by the system.
                    // Alt-Space (window context menu).
                    // Alt-F4    (close window) - actually handled in SYSKEYDOWN, but leaving here just in case.
                    if( lParam != ' ' &&
                        lParam != VK_F4
                      )
                    {
                        return 0;
                    }
            }
        }
        break;

    case WM_CLOSE:
        {
#if MYFW_EDITOR
            g_pGameCore->RequestClose();
#else
            PostQuitMessage( 0 );
#endif
        }
        return 0;

    case WM_SETFOCUS:
        {
            g_pGameCore->OnFocusGained();
            g_OnlyUpdateOnEvents = false;
        }
        break;

    case WM_KILLFOCUS:
        {
            g_OnlyUpdateOnEvents = true;

            if( g_pGameCore )
                g_pGameCore->OnFocusLost();

            for( int i=0; i<256; i++ )
                g_KeyStates[i] = false;

            for( int i=0; i<3; i++ )
                g_MouseButtonStates[i] = 0;

            if( g_SystemMouseIsLocked )
            {
                UnlockSystemMouse();
            }
        }
        break;

    case WM_CHAR:
        {
            g_pGameCore->OnChar( (unsigned int)wParam );
        }
        return 0;

    case WM_KEYDOWN:
        {
            if( wParam == VK_OEM_COMMA )
                g_KeyStates[','] = true;
            else if( wParam == VK_OEM_PERIOD )
                g_KeyStates['.'] = true;
            else if( wParam == VK_OEM_4 )
                g_KeyStates['['] = true;
            else if( wParam == VK_OEM_6 )
                g_KeyStates[']'] = true;
            else if( wParam == VK_DELETE ) // Main delete key or '.'/delete key on numpad.
                g_KeyStates[MYKEYCODE_DELETE] = true;
            else
                g_KeyStates[wParam] = true;
        }
        return 0;

    case WM_KEYUP:
        {
            if( wParam == VK_OEM_COMMA )
                g_KeyStates[','] = false;
            else if( wParam == VK_OEM_PERIOD )
                g_KeyStates['.'] = false;
            else if( wParam == VK_OEM_4 )
                g_KeyStates['['] = false;
            else if( wParam == VK_OEM_6 )
                g_KeyStates[']'] = false;
            else if( wParam == VK_DELETE ) // Delete or '.' on numpad.
                g_KeyStates[MYKEYCODE_DELETE] = false;
            else
                g_KeyStates[wParam] = false;
        }
        return 0;

    case WM_SYSKEYDOWN:
        {
            // Key presses when alt is held.
            if( wParam == VK_LMENU || wParam == VK_MENU )
                g_KeyStates[MYKEYCODE_LALT] = true;
            else if( wParam == VK_RMENU )
                g_KeyStates[MYKEYCODE_RALT] = true;
            else if( wParam == VK_F4 && (lParam & (1 << 29)) )
                break; // Let Alt-F4 pass through.
            else
                g_KeyStates[wParam] = true;
        }
        return 0;

    case WM_SYSKEYUP:
        {
            // Key releases when alt is released.
            if( wParam == VK_LMENU || wParam == VK_MENU )
                g_KeyStates[MYKEYCODE_LALT] = false;
            else if( wParam == VK_RMENU )
                g_KeyStates[MYKEYCODE_RALT] = false;
            else if( wParam == VK_F4 && (lParam & (1 << 29)) )
                break; // Since we ignored the down, let's ignore the up.
            else
                g_KeyStates[wParam] = false;
        }
        return 0;

    case WM_LBUTTONDOWN:
        {
            // Tell Windows we want mouse messages (i.e. mouse button up) from outside the window.
            SetCapture( hWnd );

            g_MouseButtonStates[0] = true;
        }
        return 0;

    case WM_LBUTTONUP:
        {
            g_MouseButtonStates[0] = false;

            // If all 3 mouse buttons are up, release capture.
            if( g_MouseButtonStates[0] == false && g_MouseButtonStates[1] == false && g_MouseButtonStates[2] == false )
            {
                ReleaseCapture();
            }
        }
        return 0;

    case WM_RBUTTONDOWN:
        {
            // Tell Windows we want mouse messages (i.e. mouse button up) from outside the window.
            SetCapture( hWnd );

            g_MouseButtonStates[1] = true;
        }
        return 0;

    case WM_RBUTTONUP:
        {
            g_MouseButtonStates[1] = false;

            // If all 3 mouse buttons are up, release capture.
            if( g_MouseButtonStates[0] == false && g_MouseButtonStates[1] == false && g_MouseButtonStates[2] == false )
            {
                ReleaseCapture();
            }
        }
        return 0;

    case WM_MBUTTONDOWN:
        {
            // Tell Windows we want mouse messages (i.e. mouse button up) from outside the window.
            SetCapture( hWnd );

            g_MouseButtonStates[2] = true;
        }
        return 0;

    case WM_MBUTTONUP:
        {
            g_MouseButtonStates[2] = false;

            // If all 3 mouse buttons are up, release capture.
            if( g_MouseButtonStates[0] == false && g_MouseButtonStates[1] == false && g_MouseButtonStates[2] == false )
            {
                ReleaseCapture();
            }
        }
        return 0;

    case WM_MOUSEWHEEL:
        {
            g_MouseWheelDelta += GET_WHEEL_DELTA_WPARAM( wParam );
        }
        return 0;

    case WM_INPUT:
        {
            unsigned int size = sizeof( RAWINPUT );
            RAWINPUT rawinput;

#if _DEBUG
            unsigned int sizewanted;
            GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, 0, &sizewanted, sizeof(RAWINPUTHEADER) );
            MyAssert( size == sizewanted );
#endif

            GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, &rawinput, &size, sizeof(RAWINPUTHEADER) );

            // Pass mouse position diffs to event queue.
            if( rawinput.header.dwType == RIM_TYPEMOUSE )
            {
                // Accumulate all raw mouse messages until they get used at the start of the next frame.
                if( g_SystemMouseIsLocked )
                {
                    //LOGInfo( "RawMouse", "%f (%d, %d)\n", MyTime_GetSystemTime(), rawinput.data.mouse.lLastX, rawinput.data.mouse.lLastY );

                    g_RawMouseDelta.x += (float)rawinput.data.mouse.lLastX;
                    g_RawMouseDelta.y += (float)rawinput.data.mouse.lLastY;
                }
            }
        }
        return 0;

    case WM_SIZE:
        {
            ResizeGLScene( LOWORD(lParam), HIWORD(lParam) );
        }
        return 0;

    case WM_DROPFILES:
        {
            HDROP hDrop = (HDROP)wParam;
            
            char filename[MAX_PATH];
            unsigned int filecount = DragQueryFileA( hDrop, -1, (LPSTR)filename, MAX_PATH );

            for( unsigned int i=0; i<filecount; i++ )
            {
                DragQueryFileA( hDrop, i, filename, MAX_PATH );

                g_pGameCore->OnDropFile( filename );
            }

            DragFinish( hDrop );
        }
        return 0;
    }

    // Pass all unhandled messages to DefWindowProc.
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

void RequestRawMouseAccess()
{
    // Only attempt this once.
    MyAssert( g_RawMouseInputInitialized == false );

    // Request raw mouse access.
    RAWINPUTDEVICE device;

    device.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
    device.usUsage = 0x02;     // HID_USAGE_GENERIC_MOUSE
    device.dwFlags = 0;
    device.hwndTarget = 0;     // All windows in this app?

    BOOL ret = RegisterRawInputDevices( &device, 1, sizeof(RAWINPUTDEVICE) );
    if( ret == false )
    {
        // Registration failed. Call GetLastError for the cause of the error.
        LOGError( LOGTag, "Failed to grab raw mouse device: %d\n", GetLastError() );
    }
    else
    {
        g_RawMouseInputInitialized = true;
    }
}

int MYFWWinMain(GameCore* pGameCore, int width, int height)
{
#if _DEBUG && MYFW_WINDOWS
    OverrideJSONMallocFree();
#endif

    RequestRawMouseAccess();

    g_EscapeButtonWillQuit = false;
    g_CloseProgramRequested = false;

    g_RequestedWidth = width;
    g_RequestedHeight = height;

    g_InitialWidth = width;
    g_InitialHeight = height;

    // Horrid key handling.
    for( int i=0; i<256; i++ )
        g_KeyStates[i] = false;

    // Initialize sockets.
    WSAData wsaData;
    int code = WSAStartup( MAKEWORD(1, 1), &wsaData );
    if( code != 0 )
    {
        LOGError( LOGTag, "WSAStartup error: %d\n",code );
        return 0;
    }

    // Create Our OpenGL Window.
    if( !CreateGLWindow( "OpenGL Window", width, height, 32, 24, 8, false ) )
    {
        return 0;
    }

    // Initialize OpenGL Extensions, must be done after OpenGL Context is created.
    OpenGL_InitExtensions();
    WGL_InitExtensions();

    // Create and initialize our Game object.
    pGameCore->OnSurfaceCreated();
    pGameCore->OnSurfaceChanged( 0, 0, width, height );
    pGameCore->OneTimeInit();

    double lastTime = MyTime_GetSystemTime();

    MSG windowsMessage;
    bool quitMessageSent = false;

    // Main loop.
    while( quitMessageSent == false )
    {
        if( PeekMessage( &windowsMessage, 0, 0, 0, PM_REMOVE ) )
        {
            if( windowsMessage.message == WM_QUIT )
            {
                quitMessageSent = true;
            }
            else
            {
                TranslateMessage( &windowsMessage );
                DispatchMessage( &windowsMessage );
            }
        }
        else
        {
            if( g_OnlyUpdateOnEvents )
            {
                WaitMessage();
                continue;
            }

            double currentTime = MyTime_GetSystemTime();
            float deltaTime = (float)(currentTime - lastTime);
            lastTime = currentTime;

            if( g_WindowIsVisible )
            {
                if( g_CloseProgramRequested || pGameCore->HasGameConfirmedCloseIsOkay() )
                {
                    quitMessageSent = true;
                }
                else
                {
                    GenerateKeyboardEvents( pGameCore );
                    GenerateMouseEvents( pGameCore );

                    pGameCore->OnDrawFrameStart( 0 );
                    g_UnpausedTime += pGameCore->Tick( deltaTime );
                    pGameCore->OnDrawFrame( 0 );
                    pGameCore->OnDrawFrameDone();

                    {
                        char tempname[MAX_PATH];
                        if( h_takescreenshot || h_moviemode )
                        {
                            h_takescreenshot = false;

                            double timeBefore = MyTime_GetSystemTime();

                            CreateDirectory( "!Screenshots", nullptr );
                            sprintf_s( tempname, MAX_PATH, "!Screenshots/Screenshot_%f", MyTime_GetSystemTime() );
                            SaveScreenshot( g_WindowWidth, g_WindowHeight, tempname );

                            double timeafter = MyTime_GetSystemTime();
                            lastTime += timeafter - timeBefore;
                        }
                    }

                    SwapBuffers( g_hDeviceContext );

                    //// Limit framerate.
                    //if( false )
                    //{
                    //    int targetFramerate = 30;
                    //    double renderTime = MyTime_GetSystemTime() - lastTime;
                    //    if( renderTime*1000 < 1000.0f/targetFramerate )
                    //    {
                    //        DWORD delay = (DWORD)( 1000.0f/targetFramerate - renderTime*1000 );
                    //        if( delay > 1000.0f/targetFramerate )
                    //            delay = 0;
                    //        //LOGInfo( LOGTag, "Sleep( %d ) - renderTime %0.2f\n", (int)delay, renderTime*1000 );
                    //        Sleep( delay );
                    //    }
                    //}
                }
            }
        }
    }

    pGameCore->OnPrepareToDie();

    KillGLWindow( true );

    WSACleanup();

    return (int)windowsMessage.wParam;
}
