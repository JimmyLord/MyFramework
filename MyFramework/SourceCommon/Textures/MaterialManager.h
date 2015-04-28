//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MaterialManager_H__
#define __MaterialManager_H__

class MaterialManager;
class TextureDefinition;

extern MaterialManager* g_pMaterialManager;

class MaterialDefinition : public CPPListNode, public RefCount
{
    friend class MaterialManager;

    static const int MAX_MATERIAL_NAME_LEN = 32;

public: // for now.
    bool m_UnsavedChanges;

    char m_Name[MAX_MATERIAL_NAME_LEN];
    MyFileObject* m_pFile;

    ShaderGroup* m_pShaderGroup;
    TextureDefinition* m_pTextureColor;
    ColorByte m_Tint;
    ColorByte m_SpecColor;
    float m_Shininess;

public:
    bool m_FullyLoaded;

public:
    MaterialDefinition();
    MaterialDefinition(ShaderGroup* pShader);
    MaterialDefinition(ShaderGroup* pShader, ColorByte tint);
    void Init();

    virtual ~MaterialDefinition();

    void ImportFromFile();

    const char* GetName() { return m_Name; }

    void SetShader(ShaderGroup* pShader);
    void SetTextureColor(TextureDefinition* pTexture);

public:
#if MYFW_USING_WX
    static void StaticOnLeftClick(void* pObjectPtr) { ((MaterialDefinition*)pObjectPtr)->OnLeftClick(); }
    void OnLeftClick();
    static void StaticOnRightClick(void* pObjectPtr) { ((MaterialDefinition*)pObjectPtr)->OnRightClick(); }
    void OnRightClick();
    static void StaticOnDrag(void* pObjectPtr) { ((MaterialDefinition*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDropShader(void* pObjectPtr) { ((MaterialDefinition*)pObjectPtr)->OnDropShader(); }
    void OnDropShader();
    static void StaticOnDropTexture(void* pObjectPtr) { ((MaterialDefinition*)pObjectPtr)->OnDropTexture(); }
    void OnDropTexture();

    void SaveMaterial();
#endif //MYFW_USING_WX
};

class MaterialManager
#if MYFW_USING_WX
: public wxEvtHandler
#endif
{
public:
    CPPListHead m_Materials;
    CPPListHead m_MaterialsStillLoading;

public:
    MaterialManager();
    virtual ~MaterialManager();

    void Tick();
    
    void FreeAllMaterials();
    
    void LoadMaterial(const char* filename);
#if MYFW_USING_WX
    void SaveAllMaterials(bool saveunchanged = false);
#endif

    MaterialDefinition* CreateMaterial(const char* name);
    MaterialDefinition* CreateMaterial(MyFileObject* pFile);
    MaterialDefinition* FindMaterial(ShaderGroup* m_pShaderGroup, TextureDefinition* pTextureColor);
    MaterialDefinition* FindMaterialByFilename(const char* filename);

public:
#if MYFW_USING_WX
    static void StaticOnLeftClick(void* pObjectPtr) { ((MaterialManager*)pObjectPtr)->OnLeftClick(); }
    void OnLeftClick();
    
    static void StaticOnRightClick(void* pObjectPtr) { ((MaterialManager*)pObjectPtr)->OnRightClick(); }
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((MaterialManager*)pObjectPtr)->OnDrag(); }
    void OnDrag();
#endif
};

#endif //__MaterialManager_H__
