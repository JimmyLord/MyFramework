//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ResourceManagers_H__
#define __ResourceManagers_H__

class BufferManager;
class EventManager;
class EventTypeManager;
class FileManager;
class FontManager;
class GamepadManager;
class GameServiceManager;
class MyJobManager;
class LightManager;
class MaterialManager;
class MeshManager;
class ShaderManager;
class ShaderGroupManager;
class SoundManager;
class TextureManager;
class VertexFormatManager;

class ResourceManagers
{
public:
    BufferManager* m_pBufferManager;
    EventManager* m_pEventManager;
    EventTypeManager* m_pEventTypeManager;
    FileManager* m_pFileManager;
    FontManager* m_pFontManager;
    GamepadManager* m_pGamepadManager;
    GameServiceManager* m_pGameServiceManager;
    MyJobManager* m_pMyJobManager;
    LightManager* m_pLightManager;
    MaterialManager* m_pMaterialManager;
    MeshManager* m_pMeshManager;
    ShaderManager* m_pShaderManager;
    ShaderGroupManager* m_pShaderGroupManager;
    SoundManager* m_pSoundManager;
    TextureManager* m_pTextureManager;
    VertexFormatManager* m_pVertexFormatManager;

public:
    ResourceManagers();
};

#endif //__ResourceManagers_H__
