//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

MessageLogCallbackFunction g_pMessageLogCallbackFunction = 0;

#if MYFW_WINDOWS || (MYFW_WP8 && _DEBUG)

void LOGInfo(const char* tag, const char* message, ...)
{
#define MAX_MESSAGE 10024
    char szBuff[MAX_MESSAGE];
    va_list arg;
    va_start(arg, message);
    vsnprintf_s( szBuff, sizeof(szBuff), _TRUNCATE, message, arg );
    va_end(arg);

    szBuff[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear
    OutputDebugStringA( szBuff );

    if( g_pMessageLogCallbackFunction != 0 )
    {
        g_pMessageLogCallbackFunction( 0, tag, szBuff );
    }
}

void LOGError(const char* tag, const char* message, ...)
{
#define MAX_MESSAGE 10024
    char szBuff[MAX_MESSAGE];
    va_list arg;
    va_start(arg, message);
    vsnprintf_s( szBuff, sizeof(szBuff), _TRUNCATE, message, arg );
    va_end(arg);

    szBuff[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear
    OutputDebugStringA( szBuff );

    if( g_pMessageLogCallbackFunction != 0 )
    {
        g_pMessageLogCallbackFunction( 1, tag, szBuff );
    }
}

void LOGDebug(const char* tag, const char* message, ...)
{
#define MAX_MESSAGE 10024
    char szBuff[MAX_MESSAGE];
    va_list arg;
    va_start(arg, message);
    vsnprintf_s( szBuff, sizeof(szBuff), _TRUNCATE, message, arg );
    va_end(arg);

    szBuff[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear
    OutputDebugStringA( szBuff );

    if( g_pMessageLogCallbackFunction != 0 )
    {
        g_pMessageLogCallbackFunction( 2, tag, szBuff );
    }
}

#elif MYFW_NACL || MYFW_BLACKBERRY || MYFW_BADA || MYFW_IOS || MYFW_OSX || MYFW_EMSCRIPTEN || MYFW_LINUX

#if MYFW_NACL
#include "../../SourceNaCL/MainInstance.h"
#include <stdio.h>
#endif

#if _DEBUG || MYFW_USING_WX

void LOGInfo(const char* tag, const char* message, ...)
{
    // TODO: watch for buffer overruns... NaCL doesn't have the _s version of vsnprintf
#define MAX_MESSAGE 10024
    char szBuff[MAX_MESSAGE];
    va_list arg;
    va_start(arg, message);
    vsnprintf( szBuff, sizeof(szBuff), message, arg );
    va_end(arg);

    szBuff[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear
#if MYFW_EMSCRIPTEN || MYFW_LINUX
    printf( "%s", szBuff );
#elif MYFW_NACL && !MYFW_PPAPI
    fprintf( stdout, "%s", szBuff );
    //g_pInstance->PostMessage( pp::Var( szBuff ) );
#elif MYFW_BLACKBERRY
    fprintf( stderr, "%s", szBuff ); // stdout doesn't go to console on BB10, so put this here.
#else
    fprintf( stdout, "%s", szBuff );
#endif

    if( g_pMessageLogCallbackFunction != 0 )
    {
        g_pMessageLogCallbackFunction( 0, tag, szBuff );
    }
}

void LOGError(const char* tag, const char* message, ...)
{
    // TODO: watch for buffer overruns... NaCL doesn't have the _s version of vsnprintf
#define MAX_MESSAGE 10024
    char szBuff[MAX_MESSAGE];
    va_list arg;
    va_start(arg, message);
    vsnprintf( szBuff, sizeof(szBuff), message, arg );
    va_end(arg);

    szBuff[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear
#if MYFW_EMSCRIPTEN || MYFW_LINUX
    printf( "%s", szBuff );
#elif MYFW_NACL && !MYFW_PPAPI
    fprintf( stderr, "%s", szBuff );
    //g_pInstance->PostMessage( pp::Var( szBuff ) );
#else
    fprintf( stderr, "%s", szBuff );
#endif

    if( g_pMessageLogCallbackFunction != 0 )
    {
        g_pMessageLogCallbackFunction( 1, tag, szBuff );
    }
}

void LOGDebug(const char* tag, const char* message, ...)
{
    // TODO: watch for buffer overruns... NaCL doesn't have the _s version of vsnprintf
#define MAX_MESSAGE 10024
    char szBuff[MAX_MESSAGE];
    va_list arg;
    va_start(arg, message);
    vsnprintf( szBuff, sizeof(szBuff), message, arg );
    va_end(arg);

    szBuff[MAX_MESSAGE-1] = 0; // vsnprintf_s might do this, but docs are unclear
#if MYFW_EMSCRIPTEN || MYFW_LINUX
    printf( "%s", szBuff );
#elif MYFW_NACL && !MYFW_PPAPI
    fprintf( stdout, "%s", szBuff );
    //g_pInstance->PostMessage( pp::Var( szBuff ) );
#elif MYFW_BLACKBERRY
    fprintf( stderr, "%s", szBuff ); // stdout doesn't go to console on BB10, so put this here.
#else
    fprintf( stdout, "%s", szBuff );
#endif

    if( g_pMessageLogCallbackFunction != 0 )
    {
        g_pMessageLogCallbackFunction( 2, tag, szBuff );
    }
}

#endif //_DEBUG

#endif
