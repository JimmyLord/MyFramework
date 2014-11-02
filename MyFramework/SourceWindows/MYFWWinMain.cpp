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

#include "CommonHeader.h"
#include "Screenshot.h"

// Initialize opengl window on windows, huge chunks taken from nehe
//    http://nehe.gamedev.net/tutorial/creating_an_opengl_window_%28win32%29/13001/

bool g_EscapeButtonWillQuit;
bool g_CloseProgramRequested;

int g_RequestedWidth;
int g_RequestedHeight;

int g_InitialWidth;
int g_InitialHeight;

HGLRC hRenderingContext = 0;
HDC hDeviceContext = 0;
HWND hWnd = 0;
HINSTANCE hInstance;

int g_WindowWidth = 0;
int g_WindowHeight = 0;
bool g_KeyStates[256];
bool g_MouseButtonStates[3];
bool g_WindowIsActive = true;
bool g_FullscreenMode = true;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static bool h_moviemode = false;
static bool h_takescreenshot = false;

bool MYFW_GetKey(int value)
{
    assert( value >= 0 && value < 256 );
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

    DWORD dwStyle = GetWindowLongPtr( hWnd, GWL_STYLE ) ;
    DWORD dwExStyle = GetWindowLongPtr( hWnd, GWL_EXSTYLE ) ;
    HMENU menu = GetMenu( hWnd ) ;

    // calculate the full size of the window needed to match our client area of width/height
    RECT WindowRect = { 0, 0, width, height } ;
    AdjustWindowRectEx( &WindowRect, dwStyle, menu ? TRUE : FALSE, dwExStyle );

    int windowwidth = WindowRect.right - WindowRect.left;
    int windowheight = WindowRect.bottom - WindowRect.top;
    
    SetWindowPos( hWnd, 0, 0, 0, windowwidth, windowheight, SWP_NOZORDER | SWP_NOMOVE ) ;
    
    ResizeGLScene( width, height );
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
            if( i == MYKEYCODE_BACKSPACE )
                pGameCore->OnKeyDown( i, i );
            else if( i == MYKEYCODE_ESC )
            {
                if( g_EscapeButtonWillQuit )
                    g_CloseProgramRequested = true;

                pGameCore->OnButtons( GCBA_Down, GCBI_Back );
                //pGameCore->OnKeyDown( 27, 27 );
            }
            else if( i == MYKEYCODE_LEFT )
                pGameCore->OnKeyDown( 37, 37 );
            else if( i == MYKEYCODE_UP )
                pGameCore->OnKeyDown( 38, 38 );
            else if( i == MYKEYCODE_RIGHT )
                pGameCore->OnKeyDown( 39, 39 );
            else if( i == MYKEYCODE_DOWN )
                pGameCore->OnKeyDown( 40, 40 );
            else if( i >= 'A' && i <= 'Z' && keys[MYKEYCODE_LSHIFT] == 0 && keys[MYKEYCODE_RSHIFT] == 0 )
                pGameCore->OnKeyDown( i+32, i+32 );
            else if( keys[MYKEYCODE_LCTRL] == 0 && keys[MYKEYCODE_RCTRL] == 0 && keys[MYKEYCODE_LALT] == 0 && keys[MYKEYCODE_RALT] == 0 )
                pGameCore->OnKeyDown( i, i );

            //LOGInfo( LOGTag, "Calling pGameCore->OnKeyDown( %d, %d )\n", i, i );
        }

        if( keys[i] == 0 && keysold[i] == 1 )
        {
            if( i == MYKEYCODE_BACKSPACE )
                pGameCore->OnKeyUp( i, i );
            else if( i == MYKEYCODE_ESC )
            {
                pGameCore->OnButtons( GCBA_Up, GCBI_Back );
                //pGameCore->OnKeyUp( 27, 27 );
            }
            else if( i == MYKEYCODE_LEFT )
                pGameCore->OnKeyUp( 37, 37 );
            else if( i == MYKEYCODE_UP )
                pGameCore->OnKeyUp( 38, 38 );
            else if( i == MYKEYCODE_RIGHT )
                pGameCore->OnKeyUp( 39, 39 );
            else if( i == MYKEYCODE_DOWN )
                pGameCore->OnKeyUp( 40, 40 );
            else if( i >= 'A' && i <= 'Z' && keys[MYKEYCODE_LSHIFT] == 0 && keys[MYKEYCODE_RSHIFT] == 0 )
                pGameCore->OnKeyUp( i+32, i+32 );
            else if( keys[MYKEYCODE_LCTRL] == 0 && keys[MYKEYCODE_RCTRL] == 0 && keys[MYKEYCODE_LALT] == 0 && keys[MYKEYCODE_RALT] == 0 )
                pGameCore->OnKeyUp( i, i );
        }
    }

    if( keys[MYKEYCODE_LCTRL] && keys['M'] == 1 && keysold['M'] == 0 ) // new press
        h_moviemode = !h_moviemode;
    if( keys[MYKEYCODE_LCTRL] && keys['S'] == 1 && keysold['S'] == 0 ) // new press
        h_takescreenshot = true;

    if( keys[MYKEYCODE_LCTRL] && keys['I'] == 1 && keysold['I'] == 0 ) // new press
    {
        if( g_pShaderManager )
            g_pShaderManager->InvalidateAllShaders( true );
        if( g_pTextureManager )
            g_pTextureManager->InvalidateAllTextures( true );
        if( g_pBufferManager )
            g_pBufferManager->InvalidateAllBuffers( true );
    }

    if( keys['1'] == 1 && keysold['1'] == 0 )
    {
        SetWindowSize( g_RequestedWidth, g_RequestedHeight );
    }
    if( keys['2'] == 1 && keysold['2'] == 0 )
    {
        SetWindowSize( g_InitialWidth, (int)(g_InitialWidth*1.5f) );
    }
    if( keys['3'] == 1 && keysold['3'] == 0 )
    {
        SetWindowSize( g_InitialHeight, g_InitialHeight );
    }
    if( keys['4'] == 1 && keysold['4'] == 0 )
    {
        SetWindowSize( (int)(g_InitialWidth*1.5f), g_InitialWidth );
    }
}

void GetMouseCoordinates(int* mx, int* my)
{
    extern HWND hWnd;

    POINT p;
    if( GetCursorPos( &p ) )
    {
        if( ScreenToClient( hWnd, &p ) )
        {
            *mx = p.x;
            *my = p.y;
        }
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

    int px;
    int py;
    GetMouseCoordinates( &px, &py );
    //int w;
    //int h;
    //glfwGetWindowSize( &w, &h );

    //float x = ((float)px*480)/w; // map the input to an x/y range of 480x800, starting at top-left
    //float y = ((float)py*800)/h;

    // button/finger 0
    if( buttons[0] == 1 && buttonsold[0] == 0 )
        pGameCore->OnTouch( GCBA_Down, 0, (float)px, (float)py, 0, 0 ); // new press

    if( buttons[0] == 0 && buttonsold[0] == 1 )
        pGameCore->OnTouch( GCBA_Up, 0, (float)px, (float)py, 0, 0 ); // new release

    if( buttons[0] == 1 && buttonsold[0] == 1 )
        pGameCore->OnTouch( GCBA_Held, 0, (float)px, (float)py, 0, 0 ); // still pressed
}

GLvoid KillGLWindow()
{
    if( g_FullscreenMode )
    {
        ChangeDisplaySettings( 0, 0 );
        ShowCursor( true );
    }

    if( hRenderingContext )
    {
        if( !wglMakeCurrent( 0, 0 ) )
        {
            MessageBox( 0, L"Release Of Device Context And Rendering Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        }

        if( !wglDeleteContext( hRenderingContext ) )
        {
            MessageBox( 0, L"Release Rendering Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        }
        
        hRenderingContext = 0;
    }

    if( hDeviceContext && !ReleaseDC( hWnd, hDeviceContext ) )
    {
        MessageBox( 0, L"Release Device Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        hDeviceContext = 0;
    }

    if( hWnd && !DestroyWindow( hWnd ) )
    {
        MessageBox( 0, L"Could Not Release hWnd.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        hWnd = 0;
    }

    if( !UnregisterClass( L"OpenGL", hInstance ) )
    {
        MessageBox( 0, L"Could Not Unregister Class.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        hInstance = 0;
    }
}

bool CreateGLWindow(wchar_t* title, int width, int height, char colorbits, char zbits, char stencilbits, bool fullscreenflag)
{
    GLuint PixelFormat;

    WNDCLASS wc;
    DWORD dwExStyle;
    DWORD dwStyle;

    RECT WindowRect;
    WindowRect.left = (long)0;
    WindowRect.right = (long)width;
    WindowRect.top = (long)0;
    WindowRect.bottom = (long)height;

    g_FullscreenMode = fullscreenflag;

    hInstance = GetModuleHandle( 0 );               // Grab An Instance For Our Window
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Redraw On Move, And Own DC For Window
    wc.lpfnWndProc = (WNDPROC)WndProc;              // WndProc Handles Messages
    wc.cbClsExtra = 0;                              // No Extra Window Data
    wc.cbWndExtra = 0;                              // No Extra Window Data
    wc.hInstance = hInstance;                       // Set The Instance
    wc.hIcon = LoadIcon( 0, IDI_WINLOGO );          // Load The Default Icon
    wc.hCursor = LoadCursor( 0, IDC_ARROW );        // Load The Arrow Pointer
    wc.hbrBackground = 0;                           // No Background Required For GL
    wc.lpszMenuName = 0;                            // We Don't Want A Menu
    wc.lpszClassName = L"OpenGL";                   // Set The Class Name

    if( !RegisterClass( &wc ) )                     // Attempt To Register The Window Class
    {
        MessageBox( 0, L"Failed To Register The Window Class.", L"ERROR", MB_OK|MB_ICONEXCLAMATION );
        return false;
    }

    if( g_FullscreenMode )
    {
        DEVMODE dmScreenSettings;                                   // Device Mode
        memset( &dmScreenSettings, 0, sizeof( dmScreenSettings ) ); // Makes Sure Memory's Cleared
        dmScreenSettings.dmSize = sizeof( dmScreenSettings );       // Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth  = width;                      // Selected Screen Width
        dmScreenSettings.dmPelsHeight = height;                     // Selected Screen Height
        dmScreenSettings.dmBitsPerPel = colorbits;                  // Selected Bits Per Pixel
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
        if( ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
        {
            // If The Mode Fails, Offer Two Options.  Quit Or Run In A Window.
            if( MessageBox( 0, L"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", L"", MB_YESNO|MB_ICONEXCLAMATION ) == IDYES )
            {
                g_FullscreenMode = false;
            }
            else
            {
                MessageBox( 0, L"Program Will Now Close.", L"ERROR", MB_OK|MB_ICONSTOP );
                return false;
            }
        }
    }

    if( g_FullscreenMode )
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
        ShowCursor( false );
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;
    }

    AdjustWindowRectEx( &WindowRect, dwStyle, false, dwExStyle );   // Adjust Window To True Requested Size

    if( !( hWnd = CreateWindowEx( dwExStyle,                            // Extended Style For The Window
                                  L"OpenGL",                            // Class Name
                                  title,                                // Window Title
                                  WS_CLIPSIBLINGS | WS_CLIPCHILDREN |   // Required Window Style
                                    dwStyle,                            // Selected Window Style
                                  0, 0,                                 // Window Position
                                  WindowRect.right-WindowRect.left,     // Calculate Adjusted Window Width
                                  WindowRect.bottom-WindowRect.top,     // Calculate Adjusted Window Height
                                  0,                                    // No Parent Window
                                  0,                                    // No Menu
                                  hInstance,                            // Instance
                                  0)))                                  // Don't Pass Anything To WM_CREATE
    {
        KillGLWindow();
        MessageBox( 0, L"Window Creation Error.", L"ERROR", MB_OK|MB_ICONEXCLAMATION );
        return false;
    }

    static PIXELFORMATDESCRIPTOR pfd =  // pfd Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // Size Of This Pixel Format Descriptor
        1,                              // Version Number
        PFD_DRAW_TO_WINDOW |            // Format Must Support Window
          PFD_SUPPORT_OPENGL |          // Format Must Support OpenGL
          PFD_DOUBLEBUFFER,             // Must Support Double Buffering
        PFD_TYPE_RGBA,                  // Request An RGBA Format
        colorbits,                      // Select Our Color Depth
        0, 0, 0, 0, 0, 0,               // Color Bits Ignored
        0,                              // No Alpha Buffer
        0,                              // Shift Bit Ignored
        0,                              // No Accumulation Buffer
        0, 0, 0, 0,                     // Accumulation Bits Ignored
        zbits,                          // Bits for Z-Buffer (Depth Buffer)
        stencilbits,                    // Stencil bits
        0,                              // No Auxiliary Buffer
        PFD_MAIN_PLANE,                 // Main Drawing Layer
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };

    if( !( hDeviceContext = GetDC(hWnd) ) ) // Did We Get A Device Context?
    {
        KillGLWindow();
        MessageBox( 0, L"Can't Create A GL Device Context.", L"ERROR", MB_OK|MB_ICONEXCLAMATION );
        return 0;
    }

    if( !( PixelFormat = ChoosePixelFormat(hDeviceContext, &pfd) ) ) // Did Windows Find A Matching Pixel Format?
    {
        KillGLWindow();
        MessageBox( 0, L"Can't Find A Suitable PixelFormat.", L"ERROR", MB_OK|MB_ICONEXCLAMATION );
        return 0;
    }

    if( !SetPixelFormat( hDeviceContext, PixelFormat, &pfd) ) // Are We Able To Set The Pixel Format?
    {
        KillGLWindow();
        MessageBox( 0, L"Can't Set The PixelFormat.", L"ERROR", MB_OK|MB_ICONEXCLAMATION );
        return 0;
    }

    if( !( hRenderingContext = wglCreateContext(hDeviceContext) ) ) // Are We Able To Get A Rendering Context?
    {
        KillGLWindow();
        MessageBox( 0, L"Can't Create A GL Rendering Context.", L"ERROR", MB_OK|MB_ICONEXCLAMATION );
        return 0;
    }

    if( !wglMakeCurrent( hDeviceContext, hRenderingContext ) ) // Try To Activate The Rendering Context
    {
        KillGLWindow();
        MessageBox( 0, L"Can't Activate The GL Rendering Context.", L"ERROR", MB_OK|MB_ICONEXCLAMATION );
        return 0;
    }

    ShowWindow( hWnd, SW_SHOW );     // Show The Window
    SetForegroundWindow( hWnd );     // Slightly Higher Priority
    SetFocus( hWnd );                // Sets Keyboard Focus To The Window
    ResizeGLScene( width, height );  // Set Up Our Perspective GL Screen

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
        case WM_ACTIVATE:
        {
            if( !HIWORD(wParam) )
            {
                g_WindowIsActive = true;
            }
            else
            {
                g_WindowIsActive = false;
            } 
            return 0;
        }

        case WM_SYSCOMMAND:
        {
            switch( wParam )
            {
                // don't let screensaver or monitor power save mode kick in.
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }
            break;
        }

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }

        //case WM_CHAR:
        //{
        ////    if( lParam & 1<<30 == 0 )
        ////    {
        ////        g_KeyStates[wParam] = true;
        ////    }
        ////    else
        ////    {
        ////        g_KeyStates[wParam] = false;
        ////    }
        //    return 0;
        //}

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
            else
                g_KeyStates[wParam] = true;
            return 0;
        }

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
            else
                g_KeyStates[wParam] = false;
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            g_MouseButtonStates[0] = true;
            return 0;
        }

        case WM_LBUTTONUP:
        {
            g_MouseButtonStates[0] = false;
            return 0;
        }

        case WM_SIZE:
        {
            ResizeGLScene( LOWORD(lParam), HIWORD(lParam) );
            return 0;
        }
    }

    // Pass all unhandled messages to DefWindowProc
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

int MYFWWinMain(int width, int height)
{
    g_EscapeButtonWillQuit = false;
    g_CloseProgramRequested = false;

    g_RequestedWidth = width;
    g_RequestedHeight = height;

    g_InitialWidth = width;
    g_InitialHeight = height;

    if( g_InitialWidth > g_InitialHeight )
    {
        g_InitialWidth = height;
        g_InitialHeight = width;
    }

    MSG msg;
    bool done = false;

    // Horrid key handling
    for( int i=0; i<256; i++ )
        g_KeyStates[i] = false;

    // Initialize sockets
    WSAData wsaData;
    int code = WSAStartup(MAKEWORD(1, 1), &wsaData);
    if( code != 0 )
    {
        LOGError( LOGTag, "WSAStartup error:%d\n",code );
        return 0;
    }

    // Create Our OpenGL Window
    if( !CreateGLWindow( L"OpenGL Window", width, height, 32, 31, 1, false ) )
    {
        return 0;
    }

    // Initialize OpenGL Extensions, must be done after OpenGL Context is created
    OpenGL_InitExtensions();

    // Create and initialize our Game object.
    g_pGameCore->OnSurfaceCreated();
    g_pGameCore->OnSurfaceChanged( 0, 0, width, height );
    g_pGameCore->OneTimeInit();

    double lasttime = MyTime_GetSystemTime();

    // Main loop
    while( !done )
    {
        if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
        {
            if( msg.message == WM_QUIT )
            {
                done = true;
            }
            else
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            double currtime = MyTime_GetSystemTime();
            double timepassed = currtime - lasttime;
            lasttime = currtime;

            if( g_WindowIsActive )
            {
                if( g_CloseProgramRequested )
                {
                    done = true;
                }
                else
                {
                    GenerateKeyboardEvents( g_pGameCore );
                    GenerateMouseEvents( g_pGameCore );

                    g_pGameCore->Tick( timepassed );
                    g_pGameCore->OnDrawFrame();
                    g_pGameCore->OnDrawFrameDone();

                    {
                        char tempname[MAX_PATH];
                        if( h_takescreenshot || h_moviemode )
                        {
                            h_takescreenshot = false;

                            double timebefore = MyTime_GetSystemTime();

                            CreateDirectory( L"!Screenshots", 0 );
                            sprintf_s( tempname, MAX_PATH, "!Screenshots/Screenshot_%f.bmp", MyTime_GetSystemTime() );
                            SaveScreenshot( width, height, tempname );

                            double timeafter = MyTime_GetSystemTime();
                            lasttime += timeafter - timebefore;
                        }
                    }

                    SwapBuffers( hDeviceContext );

                    // limit framerate;
                    if( 0 )
                    {
                        int targetframerate = 30;
                        double rendertime = MyTime_GetSystemTime() - lasttime;
                        if( rendertime*1000 < 1000.0f/targetframerate )
                        {
                            DWORD delay = (DWORD)( 1000.0f/targetframerate - rendertime*1000 );
                            if( delay > 1000.0f/targetframerate )
                                delay = 0;
                            //LOGInfo( LOGTag, "Sleep( %d ) - rendertime %0.2f\n", (int)delay, rendertime*1000 );
                            Sleep( delay );
                        }
                    }
                }
            }
        }
    }

    delete( g_pGameCore );
    g_pGameCore = 0;

    KillGLWindow();

    ValidateAllocations( true );

    WSACleanup();

    return msg.wParam;
}
