//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "ResourceManagers.h"

ResourceManagers::ResourceManagers()
{
    m_pBufferManager = nullptr;
    m_pEventManager = nullptr;
    m_pEventTypeManager = nullptr;
    m_pFileManager = nullptr;
    m_pFontManager = nullptr;
    m_pGamepadManager = nullptr;
    m_pGameServiceManager = nullptr;
    m_pLightManager = nullptr;
    m_pMeshManager = nullptr;
    m_pMaterialManager = nullptr;
    m_pJobManager = nullptr;
    m_pShaderGroupManager = nullptr;
    m_pShaderManager = nullptr;
    m_pSoundManager = nullptr;
    m_pTextureManager = nullptr;
    m_pVertexFormatManager = nullptr;

    // Globals to eliminate.
    // g_pLuaGameState                                  // 25
    // g_pBulletWorld                                   // 28
    // g_pEditorPrefs                                   // 68
    // //ImGui::SetCurrentContext( m_pImGuiContext )    // 2

    // Globals worked around, but should eliminate.
    // g_pGlobalLog                                     // 6
    // g_pMessageLogCallbackFunction                    // 19
    // g_pComponentSystemManager                        // 307
    // g_pGameCore                                      // 407
    // g_pEngineCore                                    // 379

    // Globals okay in the short term.
    // g_pIAPManager                                    // 14
    // g_pRenderer                                      // 208
    // g_pD3DContext                                    // 42
    // g_pEventTypeHashFunc                             // 10
    // g_pAllocationList                                // 19
    // g_pBrokenShaderString                            // 4

    // Other globals to look at.
    // g_pComponentTypeManager
    // g_pVisibilityLayerStrings
    // g_pEngineMainFrame
    // g_pRTQGlobals
    // g_pScreenManager
    // g_pLanguageTable
    // g_pWP8App
    // g_pGame
    // g_pAnimationKeyPool
    // g_pMainActivity
    // g_pAssetManager
    // g_pBMPFactoryLoader
    // g_pJavaSoundPlayer
    // g_pAndroidDeviceName
    // g_pJavaEnvironment
    // g_pIAPInterface
    // g_pInstance
    // g_pMyDataFormat
    // g_pPanelWatch
    // g_pPanelMemory
    // g_pPanelObjectList
    // g_pGLContext
    // g_pMainApp
}