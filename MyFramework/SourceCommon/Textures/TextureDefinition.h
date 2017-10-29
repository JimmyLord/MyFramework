//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __TextureDefinition_H__
#define __TextureDefinition_H__

class TextureManager;
class RefCount;
class FBODefinition;

class TextureDefinition : public CPPListNode, public RefCount
{
    friend class TextureManager;
    friend class FBODefinition;

protected:
    bool m_FreeFileFromRamWhenTextureCreated;
    bool m_ManagedByTextureManager;

protected:
    bool m_FullyLoaded;

    char m_Filename[MAX_PATH];
    MyFileObject* m_pFile;
    GLuint m_TextureID;

    unsigned int m_MemoryUsed;

    int m_Width;
    int m_Height;

    int m_MinFilter;
    int m_MagFilter;

    int m_WrapS;
    int m_WrapT;

public:
    TextureDefinition(bool freeonceloaded = false);
    virtual ~TextureDefinition();

    // TextureDefinition Getters
    bool IsFullyLoaded() { return m_FullyLoaded; }

    const char* GetFilename() { return (const char*)m_Filename; }
    MyFileObject* GetFile() { return m_pFile; }
    GLuint GetTextureID() { return m_TextureID; }

    unsigned int GetMemoryUsed() { return m_MemoryUsed; }

    int GetWidth() { return m_Width; }
    int GetHeight() { return m_Height; }

    // TextureDefinition Methods
    bool QueryFreeWhenCreated() { return m_FreeFileFromRamWhenTextureCreated; }
    
    void Invalidate(bool cleanglallocs);

    void FinishLoadingFileAndGenerateTexture();

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_UnloadFile = 1000,
        RightClick_FindAllReferences,
    };

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((TextureDefinition*)pObjectPtr)->OnRightClick(); }
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((TextureDefinition*)pObjectPtr)->OnDrag(); }
    void OnDrag();
#endif //MYFW_USING_WX
};

#endif //__TextureDefinition_H__
