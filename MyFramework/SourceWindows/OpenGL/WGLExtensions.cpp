//
// Copyright (c) 2016-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "WGLExtensions.h"

#pragma warning( push )
#pragma warning( disable : 4191 ) // Unsafe conversion from 'type of expression' to 'type required'.

PFNWGLCHOOSEPIXELFORMATARBPROC      wglChoosePixelFormatARB = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC   wglCreateContextAttribsARB = nullptr;

PFNWGLSWAPINTERVALEXTPROC           wglSwapInterval = nullptr;
PFNWGLGETSWAPINTERVALEXTPROC        wglGetSwapInterval = nullptr;

bool WGLExtensionSupported(const char* extension_name)
{
    // This is pointer to function which returns pointer to string with list of all wgl extensions.
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // Determine pointer to wglGetExtensionsStringEXT function.
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress( "wglGetExtensionsStringEXT" );

    if( strstr( _wglGetExtensionsStringEXT(), extension_name ) == NULL )
    {
        // string was not found
        return false;
    }

    // extension is supported
    return true;
}

void WGL_InitContextCreationExtensions()
{
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress( "wglChoosePixelFormatARB" );
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress( "wglCreateContextAttribsARB" );
}

void WGL_InitExtensions()
{
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress( "wglChoosePixelFormatARB" );
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress( "wglCreateContextAttribsARB" );

    if( WGLExtensionSupported( "WGL_EXT_swap_control" ) )
    {
        wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
        wglGetSwapInterval = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress( "wglGetSwapIntervalEXT" );
    }
}

#pragma warning( pop )
