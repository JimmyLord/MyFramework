//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyFramework_H__
#define __MyFramework_H__

// This file includes all headers in the MyFramework library.

//============================================================================================================
// C/C++/Library framework includes.
//============================================================================================================

#include "CommonHeader.h"

//============================================================================================================
// Core framework includes.
//============================================================================================================

#include "DataTypes/MyTypes.h"

#include "Memory/MyStackAllocator.h"

#include "Helpers/MyAssert.h"

#include "JSON/cJSONHelpers.h"

#include "DataTypes/CPPList.h"
#include "DataTypes/TCPPList.h"
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

#include "Physics/Box2DContactListener.h"
#include "Physics/Box2DWorld.h"

#include "Events/EventTypeManager.h"
#include "Events/MyEvent.h"
#include "Events/EventManager.h"

//============================================================================================================
// Platform specific Framework includes.
//============================================================================================================

#if MYFW_EDITOR
#include "../SourceEditor/EditorViewTypes.h"
#if MYFW_WINDOWS && MYFW_USING_IMGUI
#include "../SourceWindows/MYFWWinMain.h"
#endif // MYFW_WINDOWS && MYFW_USING_IMGUI
#endif // MYFW_EDITOR

#if MYFW_NACL
#include "../SourceNaCL/GLExtensions.h"
#include "../SourceNaCL/NaCLFileObject.h"
#include "../SourceNaCL/SoundPlayer.h"
#include "../SourceNaCL/SavedData.h"
#include "../SourceNaCL/NaCLWebRequest.h"
#endif

#if MYFW_BLACKBERRY
#if MYFW_BLACKBERRY10
#include "../SourceBlackBerry/IAPManager.h"
#endif //MYFW_BLACKBERRY10
#include "../SourceBlackBerry/GLExtensions.h"
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceBlackBerry/SavedData.h"
#include "Networking/WebRequest.h"
#include "../SourceBlackBerry/MediaPlayer.h"
#include "Networking/GameService_ScoreLoop.h"
#include "../SourceBlackBerry/BBM.h"
#endif

#if MYFW_BADA
#include "../SourceBada/TextureLoader.h"
#include "../SourceBada/SoundPlayer.h"
#include "../SourceBada/SavedData.h"
#include "../SourceBada/WebRequest.h"
#endif

#if MYFW_ANDROID
#include "../SourceAndroid/GLExtensions.h"
#include "../SourceAndroid/IAPManagerAndroid.h"
#include "../SourceAndroid/AndroidFileLoader.h"
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
#include "../SourceAndroid/GLExtensions.h"
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceEmscripten/SavedData.h"
#include "../SourceEmscripten/WebRequest.h"
#endif

#if MYFW_IOS
#include "../SourceIOS/IAPManagerIOS.h"
#include "../SourceIOS/IOSFileLoader.h"
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceIOS/SavedData.h"
#include "Networking/WebRequest.h"
#include "../SourceIOS/GameCenter.h"
#endif

#if MYFW_OSX
#include "../SourceIOS/IOSFileLoader.h"
//#include "../SourceWindows/TextureLoader.h"
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceIOS/SavedData.h"
//#include "../SourceWindows/SavedData.h"
#include "Networking/WebRequest.h"
//#include "../SourceIOS/GameCenter.h"
#endif

#if MYFW_WP8
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
#include "Networking/WebRequest.h"
#endif

#if MYFW_LINUX
#include "../SourceLinux/GLExtensions.h"
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceLinux/SavedData.h"
#include "Networking/WebRequest.h"
#include <pthread.h>
#endif

//============================================================================================================
// Common Framework includes.
//============================================================================================================

#if MYFW_WINDOWS
#define MYFW_USEINSTANCEDPARTICLES  1
#else
#define MYFW_USEINSTANCEDPARTICLES  0
#endif

#include "Renderers/Old/OpenGLWrapper.h"
#include "Renderers/BaseClasses/Renderer_Enums.h"

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

#include "Helpers/MyFileObject.h"
#include "Shaders/MyFileObjectShader.h"
#include "Helpers/FileManager.h"
#include "Helpers/MyTime.h"
#include "Helpers/ExternalTools.h"

#include "Helpers/MyTweener.h"

#include "Shaders/VertexFormats.h"
#include "Shaders/VertexFormatManager.h"
#include "Shaders/ShaderGroup.h"
#include "Shaders/ShaderManager.h"
#include "Shaders/ListOfShaders.h"

#include "Meshes/MeshManager.h"
#include "Meshes/OBJLoader.h"
#include "Meshes/MyAnimation.h"
#include "Meshes/MyMesh.h"
#include "Meshes/MySubmesh.h"
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

#include "Textures/FBODefinition.h"
#include "Textures/TextureDefinition.h"
#include "Textures/TextureManager.h"
#include "Textures/MaterialDefinition.h"
#include "Textures/MaterialManager.h"

#include "Sprites/My2DAnimInfo.h" // requires MaterialDefinition

#include "Networking/StunClient.h"
#include "Networking/GameServiceManager.h"

#include "Particles/ParticleRenderer.h"
#include "Particles/ParticleRendererInstanced.h"

#include "Sound/SoundManager.h"

#include "Core/GameCore.h"

#if MYFW_EDITOR
#include "../SourceEditor/DragAndDropHackery.h"
#endif

#endif //__MyFramework_H__
