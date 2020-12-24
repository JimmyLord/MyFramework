//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __TextureDefinition_H__
#define __TextureDefinition_H__

#include "../Renderers/BaseClasses/Renderer_Enums.h"

class FBODefinition;
class FileManager;
class MyFileObject;
class RefCount;
class TextureManager;

class TextureDefinition : public TCPPListNode<TextureDefinition*>, public RefCount
{
    friend class TextureManager;
    friend class FBODefinition;
    friend class FBO_OpenGL;

protected:
    bool m_FreeFileFromRamWhenTextureCreated;
    bool m_ManagedByTextureManager;

protected:
    bool m_FullyLoaded;

    char m_Filename[MAX_PATH];
    MyFileObject* m_pFile;

    uint32 m_MemoryUsed;

    int m_Width;
    int m_Height;

    MyRE::MinFilters m_MinFilter;
    MyRE::MagFilters m_MagFilter;

    MyRE::WrapModes m_WrapS;
    MyRE::WrapModes m_WrapT;

    virtual void GenerateTexture(unsigned char* pImageBuffer, uint32 width, uint32 height) = 0;
    virtual void GenerateErrorTexture() = 0;

public:
    TextureDefinition(bool freeOnceLoaded = false);
    virtual ~TextureDefinition();

    // Seemingly useless wrapper of release to allow Lua to call release and avoid issues with multiple inheritance.
    void Lua_Release() { Release(); }

    // Getters.
    virtual bool IsFullyLoaded() { return m_FullyLoaded; }

    const char* GetFilename() { return (const char*)m_Filename; }
    MyFileObject* GetFile() { return m_pFile; }

    unsigned int GetMemoryUsed() { return m_MemoryUsed; }

    int GetWidth() { return m_Width; }
    int GetHeight() { return m_Height; }

    // Methods.
    bool QueryFreeWhenCreated() { return m_FreeFileFromRamWhenTextureCreated; }
    
    virtual void Invalidate(bool cleanGLAllocs) = 0;

    void FinishLoadingFileAndGenerateTexture();

public:
#if MYFW_EDITOR
    enum RightClickOptions
    {
        RightClick_UnloadFile = 1000,
        RightClick_FindAllReferences,
    };

    bool m_ShowInMemoryPanel;
    void MemoryPanel_Hide() { m_ShowInMemoryPanel = false; }

    void OnPopupClick(FileManager* pFileManager, TextureDefinition* pTexture, int id);
#endif //MYFW_EDITOR
};

#endif //__TextureDefinition_H__
