//
// Copyright (c) 2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MyGLContext.h"
#include "GLExtensions.h"
#include "WGLExtensions.h"

static bool MyGL_CreateTempGLWindow(HINSTANCE hInstance);
static bool MyGL_FailAndCleanup(const char* pMessage);
static void MyGL_KillGLWindow();

static HWND g_hWndTemp = 0;
static HDC g_hDeviceContextTemp = 0;
static HGLRC g_hRenderingContext = 0;

static bool MyGL_CreateTempGLWindow(HINSTANCE hInstance)
{
    // Create a temp window to setup GL extensions:

    // Create the window.
    g_hWndTemp = CreateWindow( "MYFWClass", "Temp window",        // Window class, Title.
                               WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // Style.
                               0, 0,                              // Position.
                               1, 1,                              // Size.
                               nullptr, nullptr,                  // Parent window, Menu.
                               hInstance, nullptr );              // Instance, Param.
    if( g_hWndTemp == nullptr )
        return MyGL_FailAndCleanup( "Failed to create temp window." );

    // Get the DC.
    g_hDeviceContextTemp = GetDC( g_hWndTemp );
    if( g_hDeviceContextTemp == nullptr )
        return MyGL_FailAndCleanup( "Failed to get the temp device context." );

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

    GLuint pixelFormat = ChoosePixelFormat( g_hDeviceContextTemp, &pfd );
    if( pixelFormat == 0 )
        return MyGL_FailAndCleanup( "Failed to find a suitable PixelFormat." );

    // Set the pixel format of the device context.
    bool result = SetPixelFormat( g_hDeviceContextTemp, pixelFormat, &pfd );
    if( result == 0 )
        return MyGL_FailAndCleanup( "Failed to set the PixelFormat." );

    // Create GL rendering context.
    g_hRenderingContext = wglCreateContext( g_hDeviceContextTemp );
    if( g_hRenderingContext == nullptr )
        return MyGL_FailAndCleanup( "Failed to create a GL rendering context." );

    // Activate.
    if( wglMakeCurrent( g_hDeviceContextTemp, g_hRenderingContext ) == false )
        return MyGL_FailAndCleanup( "Failed to activate the GL rendering context." );

    return true; // Everything worked out.
}

bool MyGL_ContextCreate(HINSTANCE hInstance, HDC hDeviceContext, unsigned char colorBits, unsigned char alphaBits, unsigned char zBits, unsigned char stencilBits, unsigned char multisampleSize)
{
    // Create a temporary GL window and context to allow us to grab
    //     the WGL extension functions needed to generate proper GL context.
    {
        if( MyGL_CreateTempGLWindow( hInstance ) == false )
            return false;

        // Initialize Windows OpenGL Extensions, must be done after OpenGL Context is created.
        WGL_InitContextCreationExtensions();

        // Destroy the temp GL window.
        MyGL_KillGLWindow();
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
            WGL_ALPHA_BITS_ARB,     alphaBits,
            WGL_DEPTH_BITS_ARB,     zBits,
            WGL_STENCIL_BITS_ARB,   stencilBits,
            WGL_SAMPLE_BUFFERS_ARB, 1,
            WGL_SAMPLES_ARB,        multisampleSize,
            0
        };

        int pixelFormats[1];
        unsigned int numFormats;
        bool status = wglChoosePixelFormatARB( hDeviceContext, pixelAttribs, nullptr, 1, pixelFormats, &numFormats );
        if( status == false || numFormats == 0 )
            return MyGL_FailAndCleanup( "Failed to find a suitable PixelFormat." );

        // Set the pixel format of the device context.
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat( hDeviceContext, pixelFormats[0], sizeof(pfd), &pfd );
        bool result = SetPixelFormat( hDeviceContext, pixelFormats[0], &pfd );
        if( result == 0 )
            return MyGL_FailAndCleanup( "Failed to set the PixelFormat." );
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

        g_hRenderingContext = wglCreateContextAttribsARB( hDeviceContext, nullptr, contextAttribs );
        if( g_hRenderingContext == nullptr )
            return MyGL_FailAndCleanup( "Failed to create a GL rendering context." );

        // Activate.
        if( wglMakeCurrent( hDeviceContext, g_hRenderingContext ) == false )
            return MyGL_FailAndCleanup( "Failed to activate the GL rendering context." );
    }

    // Initialize OpenGL Extensions, must be done after OpenGL Context is created.
    OpenGL_InitExtensions();
    WGL_InitExtensions();

    return true;
}

void MyGL_ContextDestroy()
{
}

static bool MyGL_FailAndCleanup(const char* pMessage)
{
    MyGL_KillGLWindow();
    MessageBox( 0, pMessage, "ERROR", MB_OK|MB_ICONEXCLAMATION );
    return false;
}

static void MyGL_KillGLWindow()
{
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

    if( g_hDeviceContextTemp && !ReleaseDC( g_hWndTemp, g_hDeviceContextTemp ) )
    {
        MessageBox( 0, "Release device context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
    }

    if( g_hWndTemp && !DestroyWindow( g_hWndTemp ) )
    {
        MessageBox( 0, "Could not release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
    }

    g_hRenderingContext = 0;
    g_hDeviceContextTemp = 0;
    g_hWndTemp = 0;
}
