//
// Copyright (c) 2012-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyFrameworkPCH_H__
#define __MyFrameworkPCH_H__

#define USE_OPENGL 1
#define USE_PTHREAD 1
#if MYFW_WINDOWS
#define USE_GEOMETRY_SHADER 1
#endif

//============================================================================================================
// Enable/Disable specific Windows warnings.
//============================================================================================================

#if MYFW_WINDOWS
#pragma warning( 3 : 4061 ) // enumerator in switch statement is not explicitly handled by a case label
#pragma warning( 3 : 4062 ) // enumerator 'identifier' in a switch of enum 'enumeration' is not handled
//#pragma warning( 3 : 4100 ) // unreferenced formal parameter
//#pragma warning( 3 : 4127 ) // conditional expression is constant
//#pragma warning( 3 : 4189 ) // local variable is initialized but not referenced
#pragma warning( 3 : 4191 ) // unsafe conversion from 'type of expression' to 'type required'
#pragma warning( 3 : 4242 ) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning( 3 : 4244 ) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning( 3 : 4254 ) // 'operator': conversion from 'type1' to 'type2', possible loss of data
#pragma warning( 3 : 4255 ) // 'function': no function prototype given: converting '()' to '(void)'
#pragma warning( 3 : 4263 ) // member function does not override any base class virtual member function
#pragma warning( 3 : 4264 ) // no override available for virtual member function from base 'class'; function is hidden
#pragma warning( 3 : 4265 ) // class has virtual functions, but destructor is not virtual
//#pragma warning( 3 : 4266 ) // no override available for virtual member function from base 'type'; function is hidden
#pragma warning( 3 : 4287 ) // 'operator': unsigned/negative constant mismatch
#pragma warning( 3 : 4289 ) // nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
#pragma warning( 3 : 4296 ) // 'operator': expression is always false
#pragma warning( 3 : 4302 ) // truncation from 'type1' to 'type2'
#pragma warning( 3 : 4311 ) // 'variable' : pointer truncation from 'type' to 'type'
#pragma warning( 3 : 4312 ) // 'operation' : conversion from 'type1' to 'type2' of greater size
#pragma warning( 3 : 4339 ) // 'type' : use of undefined type detected in CLR meta-data - use of this type may lead to a runtime exception
#pragma warning( 3 : 4342 ) // behavior change: 'function' called, but a member operator was called in previous versions
//vs2013 #pragma warning( 3 : 4350 ) // behavior change: 'member1' called instead of 'member2'
#pragma warning( 3 : 4355 ) // 'this' : used in base member initializer list
//vcincludes #pragma warning( 3 : 4365 ) // 'action': conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning( 3 : 4370 ) // layout of class has changed from a previous version of the compiler due to better packing
//#pragma warning( 3 : 4371 ) // layout of class may have changed from a previous version of the compiler due to better packing of member 'member'
#pragma warning( 3 : 4388 ) // signed/unsigned mismatch
#pragma warning( 3 : 4389 ) // signed/unsigned mismatch
#pragma warning( 3 : 4412 ) // 'function': function signature contains type 'type'; C++ objects are unsafe to pass between pure code and mixed or native
#pragma warning( 3 : 4431 ) // missing type specifier - int assumed. Note: C no longer supports default-int
#pragma warning( 3 : 4435 ) // 'class1' : Object layout under /vd2 will change due to virtual base 'class2'
#pragma warning( 3 : 4437 ) // dynamic_cast from virtual base 'class1' to 'class2' could fail in some contexts
#pragma warning( 3 : 4444 ) // top level '__unaligned' is not implemented in this context
#pragma warning( 3 : 4471 ) // a forward declaration of an unscoped enumeration must have an underlying type (int assumed)
#pragma warning( 3 : 4472 ) // 'identifier' is a native enum: add an access specifier (private/public) to declare a managed enum
//#pragma warning( 3 : 4510 ) // default constructor could not be generated
//vcincludes #pragma warning( 3 : 4514 ) // 'function': unreferenced inline function has been removed
#pragma warning( 3 : 4536 ) // 'type name': type-name exceeds meta-data limit of 'limit' characters
#pragma warning( 3 : 4545 ) // expression before comma evaluates to a function which is missing an argument list
#pragma warning( 3 : 4546 ) // function call before comma missing argument list
#pragma warning( 3 : 4547 ) // 'operator': operator before comma has no effect; expected operator with side-effect
#pragma warning( 3 : 4548 ) // expression before comma has no effect; expected expression with side-effect
#pragma warning( 3 : 4549 ) // 'operator': operator before comma has no effect; did you intend 'operator'?
#pragma warning( 3 : 4555 ) // expression has no effect; expected expression with side-effect
#pragma warning( 3 : 4557 ) // '__assume' contains effect 'effect'
//vcincludes #pragma warning( 3 : 4571 ) // informational: catch(�) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning( 3 : 4574 ) // 'identifier' is defined to be '0': did you mean to use '#if identifier'?
#pragma warning( 3 : 4608 ) // 'symbol1' has already been initialized by another union member in the initializer list, 'symbol2'
//#pragma warning( 3 : 4610 ) // class 'type' can never be instantiated - user defined constructor required
//#pragma warning( 3 : 4619 ) // #pragma warning: there is no warning number 'number'
#pragma warning( 3 : 4623 ) // 'derived class': default constructor could not be generated because a base class default constructor is inaccessible
//vcincludes #pragma warning( 3 : 4625 ) // 'derived class': copy constructor could not be generated because a base class copy constructor is inaccessible
//vcincludes #pragma warning( 3 : 4626 ) // 'derived class': assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning( 3 : 4628 ) // digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'
//triggered by imgui, not critical for now #pragma warning( 3 : 4640 ) // 'instance': construction of local static object is not thread-safe
//#pragma warning( 3 : 4668 ) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning( 3 : 4682 ) // 'symbol' : no directional parameter attribute specified, defaulting to [in]
#pragma warning( 3 : 4686 ) // 'user-defined type': possible change in behavior, change in UDT return calling convention
#pragma warning( 3 : 4692 ) // 'function': signature of non-private member contains assembly private native type 'native_type'
//#pragma warning( 3 : 4702 ) // unreachable code
//#pragma warning( 3 : 4710 ) // 'function': function not inlined
//TODO: look into it, was triggering a lot in release. #pragma warning( 3 : 4738 ) // storing 32-bit float result in memory, possible loss of performance
//2015? #pragma warning( 3 : 4767 ) // section name 'symbol' is longer than 8 characters and will be truncated by the linker
#pragma warning( 3 : 4786 ) // 'symbol' : object name was truncated to 'number' characters in the debug information
//#pragma warning( 3 : 4820 ) // 'bytes' bytes padding added after construct 'member_name'
#pragma warning( 3 : 4826 ) // conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior
#pragma warning( 3 : 4837 ) // trigraph detected: '??%c' replaced by '%c'
#pragma warning( 3 : 4905 ) // wide string literal cast to 'LPSTR'
#pragma warning( 3 : 4906 ) // string literal cast to 'LPWSTR'
#pragma warning( 3 : 4917 ) // 'declarator': a GUID can only be associated with a class, interface, or namespace
#pragma warning( 3 : 4928 ) // illegal copy-initialization; more than one user-defined conversion has been implicitly applied
#pragma warning( 3 : 4931 ) // we are assuming the type library was built for number-bit pointers
#pragma warning( 3 : 4946 ) // reinterpret_cast used between related classes: 'class1' and 'class2'
#pragma warning( 3 : 4962 ) // 'function': profile-guided optimizations disabled because optimizations caused profile data to become inconsistent
//vcincludes #pragma warning( 3 : 4986 ) // 'symbol': exception specification does not match previous declaration
//vcincludes #pragma warning( 3 : 4987 ) // nonstandard extension used: 'throw (...)'
//2015? #pragma warning( 3 : 4988 ) // 'symbol': variable declared outside class/function scope
//#pragma warning( disable : 4996 ) // deprecated function
#endif

//============================================================================================================
// Basic data types.
//============================================================================================================

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h> // For uintptr_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if MYFW_EDITOR
#include <algorithm>
#include <list>
#include <map>
#include <string>
#include <vector>
#endif

#if MYFW_WINDOWS
#if _MSC_VER >= 1910 // VS2017+
// At the moment, this mainly affects which XAudio2 lib we're including.
// It's prefered to use the xaudio2 from the DirectX SDK (2010) for compatibility reasons,
//   but the Windows 10 sdk's version of xaudio supports Win8+, so that's not an unreasonable fallback.
// TODO: find way to detect if DXSDK is installed and use it if it is.
#define _WIN32_WINNT 0x0602 // _WIN32_WINNT_WIN8 (Includes windows 8, 10)
#pragma comment( lib, "xaudio2.lib" )
#else
// The DirectX SDK (2010) will be required for XAudio2.
// Need 2000 for WINDOWINFO
//#define _WIN32_WINNT 0x0500 // _WIN32_WINNT_WIN2K (Includes windows 2000, XP, Vista, 7, 8, 10)
// Need XP for RAWINPUTDEVICE
#define _WIN32_WINNT 0x0501 // _WIN32_WINNT_WINXP (Includes windows XP, Vista, 7, 8, 10)
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock.h>
#include <ShellAPI.h>
typedef unsigned char byte;
#endif

#if !MYFW_WINDOWS && !MYFW_WP8
#include "../SourceNaCL/ReallyShittyCode.h"
#else
#define snprintf _snprintf
#define snprintf_s _snprintf_s
#endif

#if !MYFW_WINDOWS
#define MAX_PATH PATH_MAX
#endif

//============================================================================================================
// Base networking includes.
//============================================================================================================

#if !MYFW_WINDOWS && !MYFW_WP8 && !MYFW_NACL
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> // for gethostbyname()
#include <errno.h>
#include <fcntl.h>
#include <cstdio> // for "close" on IOS... at least.
#include <unistd.h> // for "close" on OSX... at least.
#define WSAEISCONN EISCONN
#define WSAEINPROGRESS EINPROGRESS
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEALREADY EALREADY
#define WSAEINVAL EINVAL
#define WSAEISCONN EISCONN
	#if MYFW_EMSCRIPTEN
	//#define INADDR_NONE -1
	#include <unistd.h>
	#endif
#else
typedef int socklen_t;
//#include <errno.h> // for EISCONN, EINPROGRESS, EWOULDBLOCK, EALREADY, EINVAL, EISCONN, etc...
#endif

#if MYFW_EDITOR
#if MYFW_WINDOWS && MYFW_USING_IMGUI
#include <direct.h>
#endif // MYFW_WINDOWS && MYFW_USING_IMGUI
#endif // MYFW_EDITOR

//============================================================================================================
// Platform specific includes.
//============================================================================================================

#if MYFW_ANDROID
#define MYFW_OPENGLES2 1
#undef USE_PTHREAD
#define USE_PTHREAD 0
#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <pthread.h>
//#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include "../SourceAndroid/GLExtensions.h"
#endif

#if MYFW_BADA
#include <FBase.h>
#include <FSystem.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FApp.h>
#include <FSysSystemTime.h>
#include <FGraphicsOpengl2.h>
using namespace Osp::Graphics::Opengl;
#endif

#if MYFW_BLACKBERRY
#define MYFW_OPENGLES2 1
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 1
#if MYFW_BLACKBERRY10
#define USE_SCORELOOP 0
#include <bps/bps.h>
#include <bps/paymentservice.h>
#endif //MYFW_BLACKBERRY10
#include <pthread.h>
#include <screen/screen.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <unistd.h>
#include <time.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../SourceBlackBerry/GLExtensions.h"
#endif

#if MYFW_EMSCRIPTEN
#define MYFW_OPENGLES2 1
#undef USE_PTHREAD
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 1
#include <sys/time.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../SourceAndroid/GLExtensions.h"
#endif

#if MYFW_IOS
#define MYFW_OPENGLES2 1
#define glBindVertexArray glBindVertexArrayOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#define glGenVertexArrays glGenVertexArraysOES
#define glIsVertexArray glIsVertexArrayOES
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 1
#include <pthread.h>
#include <time.h>
#include <CoreFoundation/CFDate.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#if MYFW_LINUX
#define USE_OPENAL 1
#include <GL/gl.h>
#include <GL/glext.h>
#include "../SourceLinux/GLExtensions.h"
#endif

#if MYFW_NACL
#define MYFW_OPENGLES2 1
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 0
#include <GLES2/gl2.h>
#if !MYFW_PPAPI
#include <sys/time.h>
#else //MYFW_PPAPI
#include <time.h>
#endif //MYFW_PPAPI
#include <pthread.h>
#include "ppapi/c/ppb_opengles2.h"
#include "ppapi/cpp/graphics_3d_client.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/size.h"
#include "../SourceNaCL/GLExtensions.h"
#endif

#if MYFW_OSX
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glIsVertexArray glIsVertexArrayAPPLE
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 1
#include <pthread.h>
#include <time.h>
#ifndef st_mtime
#define st_mtime st_mtimespec.tv_sec // for compatibility with Linux.
#endif
#include <CoreFoundation/CFDate.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <OpenGL/gl.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#if MYFW_WINDOWS
#define USE_LOADWAVESFROMFILESYSTEM 1 // set to 0 for SDL, 1 for XAudio
#define USE_OPENAL 0
#pragma warning( push )
#pragma warning(disable:4005) // xaudio includes urlmon.h which was already included by something earlier.
#include <xaudio2.h>
#pragma warning( pop )
#include <GL/gl.h>
#include "../SourceWindows/GLExtensions.h"
#include "../SourceWindows/WGLExtensions.h"
#endif

#if MYFW_WP8
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 0
//#include <wrl/client.h>
//#include <d3d11_1.h>
//#include <DirectXMath.h>
//#include <memory>
//#include <agile.h>
#include <WinSock2.h>
#undef WIN32
#undef USE_OPENGL
#define USE_D3D 1
#include "GraphicsWrappers/Old/DXWrapper.h"
#undef WIN32
#endif

//============================================================================================================
// Library includes.
//============================================================================================================

#include "../../Libraries/OpenSimplexInC/open-simplex-noise.h"
#include "../../Libraries/cJSON/cJSON.h"
#include "../../Libraries/mtrand/mtrand.h"

#if MYFW_WINDOWS
#if WINVER >= 0x0602
#define _TIMESPEC_DEFINED
#endif
#include "../../Libraries/pthreads-w32/include/pthread.h"
#endif

//============================================================================================================
// Core framework includes.
//============================================================================================================

#define PI 3.1415926535897932384626433832795f

#define Justify_Top         0x01
#define Justify_Bottom      0x02
#define Justify_CenterY     0x04
#define Justify_Left        0x08
#define Justify_Right       0x10
#define Justify_CenterX     0x20
#define Justify_Center      (Justify_CenterX|Justify_CenterY)
#define Justify_TopLeft     (Justify_Left|Justify_Top)
#define Justify_TopRight    (Justify_Right|Justify_Top)
#define Justify_BottomLeft  (Justify_Left|Justify_Bottom)
#define Justify_BottomRight (Justify_Right|Justify_Bottom)
#define Justify_CenterLeft  (Justify_Left|Justify_CenterY)
#define Justify_CenterRight (Justify_Right|Justify_CenterY)

#if MYFW_WINDOWS
#define MYFW_USEINSTANCEDPARTICLES  1
#else
#define MYFW_USEINSTANCEDPARTICLES  0
#endif

#include "DataTypes/CPPList.h"
#include "DataTypes/TCPPList.h"

#include "Helpers/MyAssert.h"
#include "Helpers/MyMemory.h"

#include "DataTypes/MyTypes.h"
#include "DataTypes/MyList.h"
#include "DataTypes/MyMatrix.h"
#include "DataTypes/Vector.h"

#include "Helpers/MessageLog.h"
#include "Helpers/RefCount.h"
#include "Helpers/TypeInfo.h"

#endif //__MyFrameworkPCH_H__
