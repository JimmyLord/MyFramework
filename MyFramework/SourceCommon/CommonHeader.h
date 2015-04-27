//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
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
#pragma warning(3:4061) // enumerator in switch statement is not explicitly handled by a case label
//#pragma warning(3:4100) // unreferenced formal parameter
//#pragma warning(3:4127) // conditional expression is constant
//#pragma warning(3:4189) // local variable is initialized but not referenced
#pragma warning(3:4191) // unsafe conversion from 'type of expression' to 'type required'
#pragma warning(3:4242) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(3:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(3:4263) // member function does not override any base class virtual member function
#pragma warning(3:4264) // no override available for virtual member function from base 'class'; function is hidden
#pragma warning(3:4265) // class has virtual functions, but destructor is not virtual
#pragma warning(3:4266) // no override available for virtual member function from base 'type'; function is hidden
#pragma warning(3:4302) // truncation from 'type1' to 'type2'
#pragma warning(3:4389) // signed/unsigned mismatch
//#pragma warning(3:4510) // default constructor could not be generated
//#pragma warning(3:4610) // class 'type' can never be instantiated - user defined constructor required
//#pragma warning(3:4702) // unreachable code
#pragma warning(3:4826) // conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior
#pragma warning(3:4905) // wide string literal cast to 'LPSTR'
#pragma warning(3:4906) // string literal cast to 'LPWSTR'
#pragma warning(3:4928) // illegal copy-initialization; more than one user-defined conversion has been implicitly applied
//#pragma warning(disable:4996) // deprecated function
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
#define MAX_PATH 260
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

#include "../../cJSON/cJSON.h"
#include "JSON/cJSONHelpers.h"

#include "DataTypes/CPPList.h"
#include "Helpers/RefCount.h"
#include "Helpers/MyMemory.h"
#include "Helpers/Utility.h"
#include "DataTypes/Vector.h"
#include "DataTypes/MyQuaternion.h"
#include "DataTypes/MyMatrix.h"
#include "DataTypes/Plane.h"
#include "DataTypes/ColorStructs.h"
#include "DataTypes/MyList.h"
#include "DataTypes/MyActivePool.h"

#if MYFW_WINDOWS
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
//#define GLEW_STATIC
//#include "GL/glew.h"
//#include "GL/glfw.h"
#endif

#if !MYFW_WINDOWS && !MYFW_WP8
#include "../SourceNaCL/ReallyShittyCode.h"
#endif

#if MYFW_USING_WX
#pragma warning (disable : 4996)
#include "wx/wxprec.h"
//#include "wx/wx.h"
#include "wx/aui/aui.h"
#include "wx/listctrl.h"
#include "wx/treectrl.h"
#include "wx/clrpicker.h"
#include "wx/dnd.h"
#pragma warning (default : 4996)
#include <vector>
#endif

#if MYFW_NACL
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 0
#include <GLES2/gl2.h>
#if !MYFW_PPAPI
#include <sys/time.h>
#else //MYFW_PPAPI
#include <time.h>
#endif //MYFW_PPAPI
#include "ppapi/c/ppb_opengles2.h"
#include "ppapi/cpp/graphics_3d_client.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/size.h"
#include "../SourceNaCL/GLExtensions.h"
#include "../SourceNaCL/TextureLoader.h"
#include "../SourceNaCL/SoundPlayer.h"
#include "../SourceNaCL/SavedData.h"
#include "../SourceNaCL/NaCLWebRequest.h"
#endif

#if MYFW_BLACKBERRY
#define USE_LOADWAVESFROMFILESYSTEM 1
#define USE_OPENAL 1
#if MYFW_BLACKBERRY10
#define USE_SCORELOOP 1
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
#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <pthread.h>
//#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include "../SourceAndroid/GLExtensions.h"
#include "../SourceAndroid/TextureLoader.h"
#include "../SourceAndroid/SoundPlayer.h"
#include "../SourceAndroid/UnthoughtOutAndroidHeader.h"
#include "../SourceAndroid/SavedData.h"
#include "Networking/WebRequest.h"
#endif

#if MYFW_EMSCRIPTEN
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
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
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
#define USE_LOADWAVESFROMFILESYSTEM 0
#define USE_OPENAL 0
#include "../SourceWindows/GLExtensions.h"
#if USE_OPENAL
    #include "Sound/SoundPlayerOpenAL.h"
#else
    #include "../SourceWindows/SoundPlayer.h"
#endif
#include "../SourceWindows/SavedData.h"
//#include "../SourceWindows/winpthreads/winpthreads.h"
#include "../SourceWindows/pthreads-w32-2-9-1/include/pthread.h"
#include "Networking/WebRequest.h"
#endif

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
#define close closesocket
typedef int socklen_t;
//#include <errno.h> // for EISCONN, EINPROGRESS, EWOULDBLOCK, EALREADY, EINVAL, EISCONN, etc...
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

#include "Helpers/MyFileObject.h"
#include "Shaders/MyFileObjectShader.h"
#include "Helpers/FileManager.h"
#include "Helpers/MyTime.h"
#include "Helpers/ExternalTools.h"

#include "Random/mtrand.h"

#include "Helpers/MessageLog.h"
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
#include "Fonts/BMFont.h"
#include "Fonts/FontManager.h"
#include "Textures/TextureManager.h"
#include "Textures/MaterialManager.h"
#include "Textures/FBODefinition.h"
#include "Networking/StunClient.h"
#include "Networking/GameServiceManager.h"
#include "Particles/ParticleRenderer.h"

#include "GameCore.h"

#if MYFW_USING_WX
#include "../SourceWidgets/PanelObjectList.h"
#include "../SourceWidgets/PanelMemory.h"
#include "../SourceWidgets/PanelWatch.h"
#include "../SourceWidgets/DragAndDropHackery.h"
#endif

#endif //__CommonHeader_H__
