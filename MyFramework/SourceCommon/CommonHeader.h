//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __CommonHeader_H__
#define __CommonHeader_H__

//#ifdef __WXMSW__
//    #include <wx/msw/msvcrt.h>      // redefines the new() operator
//#endif

//#if MYFW_WINDOWS && _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <malloc.h>
//#include <stdlib.h>
//#include <crtdbg.h>
//#ifndef DBG_NEW
//    #ifdef new
//        #undef new
//    #endif
//    #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//    #define new DBG_NEW
//#endif
//#endif //MYFW_WINDOWS && _DEBUG

//#include "../../../MyLocalInclude.h"

#define USE_OPENGL 1
#define USE_PTHREAD 1
#if MYFW_WINDOWS
#define USE_GEOMETRY_SHADER 1
#endif

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
#pragma warning( 3 : 4266 ) // no override available for virtual member function from base 'type'; function is hidden
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

#if MYFW_WP8
//#include <wrl/client.h>
//#include <d3d11_1.h>
//#include <DirectXMath.h>
//#include <memory>
//#include <agile.h>
#include <WinSock2.h>
#undef WIN32
#undef USE_OPENGL
#define USE_D3D 1
#include "GraphicsWrappers/DXWrapper.h"
#undef WIN32
#endif

#if !MYFW_WINDOWS
#define MAX_PATH PATH_MAX
#endif

//============================================================================================================
// Basic data types
//============================================================================================================

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h> // For uintptr_t
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "DataTypes/MyTypes.h"

#include "Memory/MyStackAllocator.h"

#include "Helpers/MyAssert.h"

#include "../../Libraries/cJSON/cJSON.h"
#include "JSON/cJSONHelpers.h"

#include "DataTypes/CPPList.h"
#include "Helpers/TypeInfo.h"
#include "Helpers/RefCount.h"
#include "Helpers/MyMemory.h"
#include "Helpers/Utility.h"
#include "DataTypes/Vector.h"
#include "DataTypes/MyQuaternion.h"
#include "DataTypes/MyMatrix.h"
#include "DataTypes/Plane.h"
#include "DataTypes/ColorStructs.h"
#include "DataTypes/MyList.h"
#include "DataTypes/MyAABounds.h"

#include "Helpers/MessageLog.h"
#include "DataTypes/MyActivePool.h"

#include "../../Libraries/Box2D/Box2D/Box2D/Box2D.h"
#include "Physics/Box2DContactListener.h"
#include "Physics/Box2DWorld.h"

#include "Events/EventTypeManager.h"
#include "Events/MyEvent.h"
#include "Events/EventManager.h"

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
#define _WIN32_WINNT 0x0500 // _WIN32_WINNT_WIN2K (Includes windows 2000, XP, Vista, 7, 8, 10)
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock.h>
#include <ShellAPI.h>
typedef unsigned char byte;
#include <gl/GL.h>
#include <gl/GLU.h>
//#define GLEW_STATIC
//#include "GL/glew.h"
//#include "GL/glfw.h"
#endif

#if !MYFW_WINDOWS && !MYFW_WP8
#include "../SourceNaCL/ReallyShittyCode.h"
#else
#define snprintf _snprintf
#define snprintf_s _snprintf_s
#endif

//============================================================================================================
// Base networking includes
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

//============================================================================================================
// Platform specific includes
//============================================================================================================

#if MYFW_USING_WX
#pragma warning( push )
#pragma warning( disable : 4191 )
#pragma warning( disable : 4265 )
#pragma warning( disable : 4574 )
#pragma warning( disable : 4996 )
#include "wx/wxprec.h"
//#include "wx/wx.h"
#include "wx/aui/aui.h"
#include "wx/listctrl.h"
#include "wx/treectrl.h"
#include "wx/clrpicker.h"
#include "wx/dnd.h"
#include "wx/menu.h"
#include "wx/artprov.h"
#pragma warning( pop )
#endif

#if MYFW_EDITOR
#if MYFW_WINDOWS && MYFW_USING_IMGUI
#include <direct.h>
#include "../SourceWindows/MYFWWinMain.h"
#endif
#include <vector>
#include <map>
#include <list>
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
#include "../SourceNaCL/NaCLFileObject.h"
#include "../SourceNaCL/SoundPlayer.h"
#include "../SourceNaCL/SavedData.h"
#include "../SourceNaCL/NaCLWebRequest.h"
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
#include "../SourceBlackBerry/IAPManager.h"
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
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceBlackBerry/SavedData.h"
#include "Networking/WebRequest.h"
#include "../SourceBlackBerry/MediaPlayer.h"
#include "Networking/GameService_ScoreLoop.h"
#include "../SourceBlackBerry/BBM.h"
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
#include "../SourceBada/TextureLoader.h"
#include "../SourceBada/SoundPlayer.h"
#include "../SourceBada/SavedData.h"
#include "../SourceBada/WebRequest.h"
#endif

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
#include "../SourceAndroid/IAPManagerAndroid.h"
#include "../SourceAndroid/TextureLoader.h"
#if 0
#include "../SourceAndroid/SoundPlayer.h"
#else
#include "../SourceAndroid/SoundPlayerOpenSL.h"
#endif
#include "../SourceAndroid/JavaInterfaceCPP.h"
#include "../SourceAndroid/SavedData.h"
#include "Networking/WebRequest.h"
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
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceEmscripten/SavedData.h"
#include "../SourceEmscripten/WebRequest.h"
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
#include "../SourceIOS/IAPManagerIOS.h"
#include "../SourceIOS/TextureLoader.h"
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceIOS/SavedData.h"
#include "Networking/WebRequest.h"
#include "../SourceIOS/GameCenter.h"
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
#include "../SourceIOS/TextureLoader.h"
//#include "../SourceWindows/TextureLoader.h"
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceIOS/SavedData.h"
//#include "../SourceWindows/SavedData.h"
#include "Networking/WebRequest.h"
//#include "../SourceIOS/GameCenter.h"
#endif

#if MYFW_WP8
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 0
#include "../SourceWP8/TextureLoader.h"
//#include "../SourceWP8/MySaveFileObject_LocalStorage.h"
#if USE_OPENAL
    #include "Sound/SoundPlayerOpenAL.h"
#else
    #include "../SourceWP8/SoundPlayer.h"
#endif
#include "../SourceWP8/SavedData.h"
#include "Networking/WebRequest.h"
#endif

#if MYFW_WINDOWS
#define USE_LOADWAVESFROMFILESYSTEM 1 // set to 0 for SDL, 1 for XAudio
#define USE_OPENAL 0
#include "../SourceWindows/GLExtensions.h"
#include "../SourceWindows/WGLExtensions.h"
#pragma warning( push )
#pragma warning(disable:4005) // xaudio includes urlmon.h which was already included by something earlier.
#include <xaudio2.h>
#pragma warning( pop )
#if USE_OPENAL
    #include "Sound/SoundPlayerOpenAL.h"
#else
    //#include "../SourceWindows/SoundPlayerSDL.h"
    #include "../SourceWindows/SoundPlayerXAudio.h"
#endif
#include "../SourceWindows/SavedData.h"
//#include "../SourceWindows/winpthreads/winpthreads.h"
#if WINVER >= 0x0602
#define _TIMESPEC_DEFINED
#endif
#include "../../Libraries/pthreads-w32/include/pthread.h"
#include "Networking/WebRequest.h"
#endif

#if MYFW_LINUX
#include <GL/gl.h>
#include <GL/glext.h>
#include "../SourceLinux/GLExtensions.h"
#define USE_OPENAL 1
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceLinux/SavedData.h"
#include "Networking/WebRequest.h"
#include <pthread.h>
#endif

//============================================================================================================
// Common framework includes
//============================================================================================================

#if MYFW_WINDOWS
#define MYFW_USEINSTANCEDPARTICLES  1
#else
#define MYFW_USEINSTANCEDPARTICLES  0
#endif

#include "GraphicsWrappers/OpenGLWrapper.h"

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

#include "Multithreading/MyThread.h"
#include "Multithreading/MyJobManager.h"

#include "Noise/SimplexNoise.h"

#include "Helpers/MyFileObject.h"
#include "Shaders/MyFileObjectShader.h"
#include "Helpers/FileManager.h"
#include "Helpers/MyTime.h"
#include "Helpers/ExternalTools.h"

#include "Random/mtrand.h"

#include "Helpers/GlobalEnums.h"
#include "Helpers/MyTweener.h"

#include "Shaders/GLHelpers.h"
#include "Shaders/VertexFormats.h"
#include "Shaders/VertexFormatManager.h"
#include "Shaders/ShaderGroup.h"
#include "Shaders/ShaderManager.h"
#include "Shaders/Shader_Base.h"
#include "Shaders/ListOfShaders.h"

#include "Meshes/MeshManager.h"
#include "Meshes/OBJLoader.h"
#include "Meshes/MyAnimation.h"
#include "Meshes/MyMesh.h"
#include "Meshes/MyMeshLoader.h"
#include "Meshes/BufferManager.h"
#include "Meshes/MyLight.h"
#include "Meshes/LightManager.h"

#include "Sprites/MySprite.h"
#include "Sprites/MySprite_XYZVertexColor.h"
//#include "Sprites/MySprite9.h"
#include "Sprites/SpriteBatch.h"
#include "Sprites/SpriteBatch_XYZVertexColor.h"
#include "Sprites/AnimatedSprite.h"
#include "Sprites/AnimationKeys.h"
#include "Sprites/SpriteSheet.h"
#include "Sprites/SpriteSheet_XYZVertexColor.h"

#include "SceneGraphs/SceneGraph_Base.h"

#include "Fonts/BMFont.h"
#include "Fonts/FontManager.h"

#include "Textures/TextureDefinition.h"
#include "Textures/TextureManager.h"
#include "Textures/MaterialDefinition.h"
#include "Textures/MaterialManager.h"
#include "Textures/FBODefinition.h"

#include "Sprites/My2DAnimInfo.h" // requires MaterialDefinition

#include "Networking/StunClient.h"
#include "Networking/GameServiceManager.h"

#include "Particles/ParticleRenderer.h"
#include "Particles/ParticleRendererInstanced.h"

#include "Sound/SoundManager.h"

#include "GameCore.h"

#if MYFW_USING_WX
#include "../SourceWidgets/MYFWMainWx.h"
#include "../SourceWidgets/PanelObjectList.h"
#include "../SourceWidgets/PanelMemory.h"
#include "../SourceWidgets/PanelWatch.h"
#endif

#if MYFW_EDITOR
#include "../SourceEditor/DragAndDropHackery.h"
#endif

#endif //__CommonHeader_H__
