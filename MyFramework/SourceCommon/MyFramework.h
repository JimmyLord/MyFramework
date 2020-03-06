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

#include "MyFrameworkPCH.h"

//============================================================================================================
// Core framework includes.
//============================================================================================================

//#include "DataTypes/MyTypes.h"

#include "Memory/MyStackAllocator.h"

#include "JSON/cJSONHelpers.h"

#include "Helpers/Utility.h"

#include "DataTypes/ColorStructs.h"
#include "DataTypes/MyAABounds.h"
#include "DataTypes/MyActivePool.h"
#include "DataTypes/MyQuaternion.h"
#include "DataTypes/Plane.h"

#if MYFW_USING_BOX2D
#include "Physics/Box2DContactListener.h"
#include "Physics/Box2DWorld.h"
#endif

#include "Events/EventTypeManager.h"
#include "Events/MyEvent.h"
#include "Events/EventManager.h"

#if MYFW_EDITOR
#include "../SourceEditor/EditorViewTypes.h"
#if MYFW_WINDOWS && MYFW_USING_IMGUI
#include "../SourceWindows/MYFWWinMain.h"
#endif // MYFW_WINDOWS && MYFW_USING_IMGUI
#endif // MYFW_EDITOR

//============================================================================================================
// Platform specific Framework includes.
//============================================================================================================

// TODO: Fix these includes so there are common base classes.

#if MYFW_ANDROID
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

#if MYFW_BADA
#include "../SourceBada/TextureLoader.h"
#include "../SourceBada/SoundPlayer.h"
#include "../SourceBada/SavedData.h"
#include "../SourceBada/WebRequest.h"
#endif

#if MYFW_BLACKBERRY
#if MYFW_BLACKBERRY10
#include "../SourceBlackBerry/IAPManager.h"
#endif //MYFW_BLACKBERRY10
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceBlackBerry/SavedData.h"
#include "Networking/WebRequest.h"
#include "../SourceBlackBerry/MediaPlayer.h"
#include "Networking/GameService_ScoreLoop.h"
#include "../SourceBlackBerry/BBM.h"
#endif

#if MYFW_EMSCRIPTEN
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

#if MYFW_LINUX
#include "Sound/SoundPlayerOpenAL.h"
#include "../SourceLinux/SavedData.h"
#include "Networking/WebRequest.h"
#include <pthread.h>
#endif

#if MYFW_NACL
#include "../SourceNaCL/NaCLFileObject.h"
#include "../SourceNaCL/SoundPlayer.h"
#include "../SourceNaCL/SavedData.h"
#include "../SourceNaCL/NaCLWebRequest.h"
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

#if MYFW_WINDOWS
#if USE_OPENAL
    #include "Sound/SoundPlayerOpenAL.h"
#else
    //#include "../SourceWindows/SoundPlayerSDL.h"
    #include "../SourceWindows/SoundPlayerXAudio.h"
#endif
#include "../SourceWindows/SavedData.h"
//#include "../SourceWindows/winpthreads/winpthreads.h"
#include "Networking/WebRequest.h"
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

//============================================================================================================
// Library includes.
//============================================================================================================

//#include "../../Libraries/Box2D/Box2D/Box2D.h"

//============================================================================================================
// Common Framework includes.
//============================================================================================================

#include "Renderers/Old/OpenGLWrapper.h"
#include "Renderers/BaseClasses/Renderer_Enums.h"

#include "Multithreading/MyThread.h"
#include "Multithreading/MyJobManager.h"

#include "Helpers/MyFileObject.h"
#include "Shaders/MyFileObjectShader.h"
#include "FileSystem/FileChangeDetector.h"
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

#include "RenderGraphs/RenderGraph_Base.h"

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
