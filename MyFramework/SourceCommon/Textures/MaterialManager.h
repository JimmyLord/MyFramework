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

typedef void (*MaterialCreatedCallbackFunc)(void* pObjectPtr, MaterialDefinition* pMaterial);
struct MaterialCreatedCallbackStruct
{
    void* pObj;
    MaterialCreatedCallbackFunc pFunc;
};

class MaterialManager
#if MYFW_USING_WX
: public wxEvtHandler
#endif
{
    static const int MAX_REGISTERED_CALLBACKS = 1; // TODO: fix this hardcodedness

public:
    CPPListHead m_Materials;
    CPPListHead m_MaterialsStillLoading;

    MyList<MaterialCreatedCallbackStruct> m_pMaterialCreatedCallbackList;

public:
    MaterialManager();
    virtual ~MaterialManager();

    void Tick();
    
    void FreeAllMaterials();

    // Callbacks
    void RegisterMaterialCreatedCallback(void* pObj, MaterialCreatedCallbackFunc pCallback);

#if MYFW_USING_WX
    void SaveAllMaterials(bool saveunchanged = false);
#endif

    MaterialDefinition* CreateMaterial(const char* name = 0);
    MaterialDefinition* LoadMaterial(const char* fullpath);
    MaterialDefinition* FindMaterial(ShaderGroup* m_pShaderGroup, TextureDefinition* pTextureColor);
    MaterialDefinition* FindMaterialByFilename(const char* fullpath);

public:
#if MYFW_USING_WX
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int index) { ((MaterialManager*)pObjectPtr)->OnLeftClick( index ); }
    void OnLeftClick(unsigned int index);
    
    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((MaterialManager*)pObjectPtr)->OnRightClick(); }
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((MaterialManager*)pObjectPtr)->OnDrag(); }
    void OnDrag();
#endif
};

#endif //__MaterialManager_H__
