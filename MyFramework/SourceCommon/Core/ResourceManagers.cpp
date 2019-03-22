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
    m_pBufferManager = nullptr;         // g_pBufferManager // 44
    m_pEventManager = nullptr;          // g_pEventManager // 24
    m_pEventTypeManager = nullptr;      // g_pEventTypeManager // 7
    m_pFileManager = nullptr;           // g_pFileManager // 97
    m_pFontManager = nullptr;           // g_pFontManager // 15
    m_pGameServiceManager = nullptr;    // g_pGameServiceManager // 14
    m_pGamepadManager = nullptr;
    m_pMyJobManager = nullptr;
    m_pLightManager = nullptr;          // g_pLightManager // 21
    m_pMaterialManager = nullptr;       // g_pMaterialManager // 58
    m_pMeshManager = nullptr;           // g_pMeshManager // 18
    m_pShaderManager = nullptr;         // g_pShaderManager // 10
    m_pShaderGroupManager = nullptr;    // g_pShaderGroupManager // 19
    m_pSoundManager = nullptr;
    m_pTextureManager = nullptr;        // g_pTextureManager // 42
    m_pVertexFormatManager = nullptr;   // g_pVertexFormatManager // 7
}