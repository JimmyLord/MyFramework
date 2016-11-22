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

#if MYFW_USING_WX

#define MYKEYCODE_BACKSPACE     8
#define MYKEYCODE_TAB           9
#define MYKEYCODE_ENTER         13
#define MYKEYCODE_ESC           27
#define MYKEYCODE_LEFT          314 //arrows  //376(numpad arrows)
#define MYKEYCODE_UP            315 //arrows  //377(numpad arrows)
#define MYKEYCODE_RIGHT         316 //arrows  //378(numpad arrows)
#define MYKEYCODE_DOWN          317 //arrows  //379(numpad arrows)
#define MYKEYCODE_LSHIFT        306
#define MYKEYCODE_RSHIFT        306
#define MYKEYCODE_LCTRL         308
#define MYKEYCODE_RCTRL         308
#define MYKEYCODE_LALT          307
#define MYKEYCODE_RALT          307
#define MYKEYCODE_DELETE        127
#define MYKEYCODE_PAGEUP        380
#define MYKEYCODE_PAGEDOWN      381
#define MYKEYCODE_HOME          375
#define MYKEYCODE_END           382

#elif MYFW_ANDROID

#define MYKEYCODE_BACKSPACE     67  //KEYCODE_DEL
#define MYKEYCODE_ENTER         66

#elif MYFW_WINDOWS

#define MYKEYCODE_BACKSPACE     8
#define MYKEYCODE_TAB           9
#define MYKEYCODE_ENTER         13
#define MYKEYCODE_ESC           27
#define MYKEYCODE_LEFT          37
#define MYKEYCODE_UP            38
#define MYKEYCODE_RIGHT         39
#define MYKEYCODE_DOWN          40
#define MYKEYCODE_LSHIFT        16
#define MYKEYCODE_RSHIFT        16
#define MYKEYCODE_LCTRL         17
#define MYKEYCODE_RCTRL         17
#define MYKEYCODE_LALT          18
#define MYKEYCODE_RALT          18
#define MYKEYCODE_DELETE        127
#define MYKEYCODE_PAGEUP        33
#define MYKEYCODE_PAGEDOWN      34
#define MYKEYCODE_HOME          36
#define MYKEYCODE_END           35

#elif MYFW_BLACKBERRY

//#include <sys/keycodes.h>
#define MYKEYCODE_BACKSPACE     61448 //KEYCODE_BACKSPACE  //KEYCODE_DEL
#define MYKEYCODE_ENTER         61453 //KEYCODE_RETURN

#elif MYFW_NACL || MYFW_EMSCRIPTEN

#elif MYFW_OSX

#define MYKEYCODE_LSHIFT        16
#define MYKEYCODE_RSHIFT        16
#define MYKEYCODE_LCTRL         17
#define MYKEYCODE_RCTRL         17

#endif

#ifndef MYKEYCODE_BACKSPACE
#define MYKEYCODE_BACKSPACE     8
#endif
#ifndef MYKEYCODE_TAB
#define MYKEYCODE_TAB           9
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

#ifndef MYKEYCODE_LSHIFT
#define MYKEYCODE_LSHIFT        16
#endif
#ifndef MYKEYCODE_RSHIFT
#define MYKEYCODE_RSHIFT        16
#endif
#ifndef MYKEYCODE_LCTRL
#define MYKEYCODE_LCTRL         17
#endif
#ifndef MYKEYCODE_RCTRL
#define MYKEYCODE_RCTRL         17
#endif
#ifndef MYKEYCODE_LALT
#define MYKEYCODE_LALT          18
#endif
#ifndef MYKEYCODE_RALT
#define MYKEYCODE_RALT          18
#endif

#ifndef MYKEYCODE_DELETE
#define MYKEYCODE_DELETE        127
#endif
#ifndef MYKEYCODE_PAGEUP
#define MYKEYCODE_PAGEUP        33
#endif
#ifndef MYKEYCODE_PAGEDOWN
#define MYKEYCODE_PAGEDOWN      34
#endif
#ifndef MYKEYCODE_HOME
#define MYKEYCODE_HOME          36
#endif
#ifndef MYKEYCODE_END
#define MYKEYCODE_END           35
#endif

#if MYFW_WP8
extern char g_TextToShare_Subject[1000];
extern char g_TextToShare_Body[1000];
#endif

void SetRenderMode(bool continuous);
void SetExitOnBackButton(bool exit);
void SetMusicVolume(float volume);
void LaunchURL(const char* url);
void ShareString(const char* subject, const char* body);
void ShowKeyboard(bool show);
bool PlatformSpecific_CheckKeyState(int keycode);
void PlatformSpecific_SetMousePosition(float x, float y);
void PlatformSpecific_SetMouseLock(bool lock);
bool PlatformSpecific_IsMouseLocked();

#endif //__ExternalTools_H__
