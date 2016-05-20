//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ExternalTools_H__
#define __ExternalTools_H__

#ifdef MYFW_ANDROID

#define MYKEYCODE_BACKSPACE     67  //KEYCODE_DEL
#define MYKEYCODE_ENTER         66

#elif MYFW_WINDOWS

#if MYFW_USING_WX
#define MYKEYCODE_LSHIFT        306
#define MYKEYCODE_RSHIFT        306
#define MYKEYCODE_LCTRL         308
#define MYKEYCODE_RCTRL         308
#define MYKEYCODE_LALT          307
#define MYKEYCODE_RALT          307
#else
#define MYKEYCODE_LSHIFT        16
#define MYKEYCODE_RSHIFT        16
#define MYKEYCODE_LCTRL         17
#define MYKEYCODE_RCTRL         17
#define MYKEYCODE_LALT          0
#define MYKEYCODE_RALT          0
#endif

#elif MYFW_BLACKBERRY

//#include <sys/keycodes.h>
#define MYKEYCODE_BACKSPACE     61448 //KEYCODE_BACKSPACE  //KEYCODE_DEL
#define MYKEYCODE_ENTER         61453 //KEYCODE_RETURN

#elif MYFW_NACL || MYFW_EMSCRIPTEN

#elif MYFW_OSX

#if MYFW_USING_WX
#define MYKEYCODE_LSHIFT        306
#define MYKEYCODE_RSHIFT        306
#define MYKEYCODE_LCTRL         308
#define MYKEYCODE_RCTRL         308
#define MYKEYCODE_LALT          307
#define MYKEYCODE_RALT          307
#else
#define MYKEYCODE_LSHIFT        16
#define MYKEYCODE_RSHIFT        16
#define MYKEYCODE_LCTRL         17
#define MYKEYCODE_RCTRL         17
#define MYKEYCODE_LALT          0
#define MYKEYCODE_RALT          0
#endif

#else

#define MYKEYCODE_BACKSPACE     0
#define MYKEYCODE_ENTER         0
#define MYKEYCODE_ESC           0
#define MYKEYCODE_LEFT          0
#define MYKEYCODE_UP            0
#define MYKEYCODE_RIGHT         0
#define MYKEYCODE_DOWN          0

#endif

#ifndef MYKEYCODE_BACKSPACE
#define MYKEYCODE_BACKSPACE     8
#endif
#ifndef MYKEYCODE_ENTER
#define MYKEYCODE_ENTER         13
#endif
#ifndef MYKEYCODE_ESC
#define MYKEYCODE_ESC           27
#endif
#ifndef MYKEYCODE_LEFT
#define MYKEYCODE_LEFT          37
#endif
#ifndef MYKEYCODE_UP
#define MYKEYCODE_UP            38
#endif
#ifndef MYKEYCODE_RIGHT
#define MYKEYCODE_RIGHT         39
#endif
#ifndef MYKEYCODE_DOWN
#define MYKEYCODE_DOWN          40
#endif
#ifndef MYKEYCODE_DELETE
#define MYKEYCODE_DELETE        127
#endif

#if MYFW_WP8
extern char g_TextToShare_Subject[1000];
extern char g_TextToShare_Body[1000];
#endif

void SetExitOnBackButton(bool exit);
void SetMusicVolume(float volume);
void LaunchURL(const char* url);
void ShareString(const char* subject, const char* body);
void ShowKeyboard(bool show);

#endif //__ExternalTools_H__
