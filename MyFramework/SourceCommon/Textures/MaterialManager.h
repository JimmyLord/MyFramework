//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MaterialManager_H__
#define __MaterialManager_H__

class MaterialDefinition;
class MaterialManager;
class TextureDefinition;
class TextureManager;

extern MaterialManager* g_pMaterialManager;

typedef void MaterialCreatedCallbackFunc(void* pObjectPtr, MaterialDefinition* pMaterial);
struct MaterialCreatedCallbackStruct
{
    void* pObj;
    MaterialCreatedCallbackFunc* pFunc;
};

class MaterialManager
#if MYFW_USING_WX
: public wxEvtHandler
#endif
{
    static const int MAX_REGISTERED_CALLBACKS = 1; // TODO: Fix this hardcodedness.

private:
#if MYFW_USING_WX
    wxTreeItemId m_TreeIDRightClicked;
#endif

protected:
    TextureManager* m_pTextureManager;

    TCPPListHead<MaterialDefinition*> m_Materials;
    TCPPListHead<MaterialDefinition*> m_MaterialsStillLoading;

    MyList<MaterialCreatedCallbackStruct> m_pMaterialCreatedCallbackList;

#if MYFW_EDITOR
    MaterialDefinition* m_pDefaultEditorMaterial;
#endif

public:
    MaterialManager(TextureManager* pTextureManager);
    virtual ~MaterialManager();

    TextureManager* GetTextureManager() { return m_pTextureManager; }

    void Tick();
    
    void FreeAllMaterials();

    // Callbacks.
    void RegisterMaterialCreatedCallback(void* pObj, MaterialCreatedCallbackFunc* pCallback);

#if MYFW_EDITOR
    void SaveAllMaterials(bool saveunchanged = false);

    MaterialDefinition* Editor_GetFirstMaterialLoaded()       { return (MaterialDefinition*)m_Materials.GetHead(); }
    MaterialDefinition* Editor_GetFirstMaterialStillLoading() { return (MaterialDefinition*)m_MaterialsStillLoading.GetHead(); }
    void Editor_MoveMaterialToFrontOfLoadedList(MaterialDefinition* pMaterial);
    MaterialDefinition* GetDefaultEditorMaterial();
#endif

    MaterialDefinition* CreateMaterial(MyFileObject* pMaterialFile);
    MaterialDefinition* CreateMaterial(const char* name = 0, const char* relativePath = 0);
    MaterialDefinition* LoadMaterial(const char* fullpath);
    void ReloadMaterial(MaterialDefinition* pMaterial); // Can only be called if file on disk changed and is being reloaded.
    MaterialDefinition* GetFirstMaterial();
    MaterialDefinition* FindMaterial(ShaderGroup* m_pShaderGroup, TextureDefinition* pTextureColor);
    MaterialDefinition* FindMaterialByFilename(const char* fullpath);

public:
#if MYFW_EDITOR
    void CallMaterialCreatedCallbacks(MaterialDefinition* pMaterial);
#endif
};

#endif //__MaterialManager_H__
