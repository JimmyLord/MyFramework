//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MessageLog_H__
#define __MessageLog_H__

#define LOGTag "Flathead"

typedef void (*MessageLogCallbackFunction)(int logtype, const char* tag, const char* message);
extern MessageLogCallbackFunction g_pMessageLogCallbackFunction;

#if MYFW_ANDROID

#if _DEBUG
#define LOGInfo(...) __android_log_print( ANDROID_LOG_INFO, __VA_ARGS__ )
#define LOGError(...) __android_log_print( ANDROID_LOG_ERROR, __VA_ARGS__ )
#else
#define LOGInfo(...)
#define LOGError(...)
#endif

#else

#if MYFW_WINDOWS || (MYFW_WP8 && _DEBUG)
void LOGInfo(const char* tag, const char* message, ...);
void LOGError(const char* tag, const char* message, ...);
void LOGDebug(const char* tag, const char* message, ...);
#elif _DEBUG && (MYFW_NACL || MYFW_BLACKBERRY || MYFW_BADA || MYFW_IOS || MYFW_OSX || MYFW_EMSCRIPTEN)
void LOGInfo(const char* tag, const char* message, ...);
void LOGError(const char* tag, const char* message, ...);
void LOGDebug(const char* tag, const char* message, ...);
#else
#define LOGInfo(...)
#define LOGError(...)
#define LOGDebug(...)
#endif

#endif

#endif //__MessageLog_H__
