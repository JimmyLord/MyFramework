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
class LightManager;
class MeshManager;
class MaterialManager;
class MyJobManager;
class ShaderGroupManager;
class ShaderManager;
class SoundManager;
class TextureManager;
class VertexFormatManager;

class ResourceManagers
{
    friend class GameCore;
    friend class EngineCore;

protected:
    BufferManager*          m_pBufferManager;
    EventManager*           m_pEventManager;
    EventTypeManager*       m_pEventTypeManager;
    FileManager*            m_pFileManager;
    FontManager*            m_pFontManager;
    GamepadManager*         m_pGamepadManager;
    GameServiceManager*     m_pGameServiceManager;
    LightManager*           m_pLightManager;
    MeshManager*            m_pMeshManager;
    MaterialManager*        m_pMaterialManager;
    MyJobManager*           m_pJobManager;
    ShaderGroupManager*     m_pShaderGroupManager;
    ShaderManager*          m_pShaderManager;
    SoundManager*           m_pSoundManager;
    TextureManager*         m_pTextureManager;
    VertexFormatManager*    m_pVertexFormatManager;

public:
    ResourceManagers();

    BufferManager*          GetBufferManager()          { return m_pBufferManager;       }
    EventManager*           GetEventManager()           { return m_pEventManager;        }
    EventTypeManager*       GetEventTypeManager()       { return m_pEventTypeManager;    }
    FileManager*            GetFileManager()            { return m_pFileManager;         }
    FontManager*            GetFontManager()            { return m_pFontManager;         }
    GamepadManager*         GetGamepadManager()         { return m_pGamepadManager;      }
    GameServiceManager*     GetGameServiceManager()     { return m_pGameServiceManager;  }
    LightManager*           GetLightManager()           { return m_pLightManager;        }
    MaterialManager*        GetMaterialManager()        { return m_pMaterialManager;     }
    MeshManager*            GetMeshManager()            { return m_pMeshManager;         }
    MyJobManager*           GetJobManager()             { return m_pJobManager;          }
    ShaderGroupManager*     GetShaderGroupManager()     { return m_pShaderGroupManager;  }
    ShaderManager*          GetShaderManager()          { return m_pShaderManager;       }
    SoundManager*           GetSoundManager()           { return m_pSoundManager;        }
    TextureManager*         GetTextureManager()         { return m_pTextureManager;      }
    VertexFormatManager*    GetVertexFormatManager()    { return m_pVertexFormatManager; }
};

#endif //__ResourceManagers_H__
