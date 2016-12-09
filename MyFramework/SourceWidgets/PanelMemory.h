//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __PanelMemory_H__
#define __PanelMemory_H__

class PanelMemory;

extern PanelMemory* g_pPanelMemory;

enum PanelMemoryPages
{
    PanelMemoryPage_Materials,
    PanelMemoryPage_Textures,
    PanelMemoryPage_ShaderGroups,
    PanelMemoryPage_SoundCues,
    PanelMemoryPage_Files,
    PanelMemoryPage_Buffers,
    PanelMemoryPage_DrawCalls,
};

class PanelMemory : public wxPanel
{
public:
    wxNotebook* m_pNotebook;
    wxTreeCtrl* m_pTree_Materials;
    wxTreeCtrl* m_pTree_Textures;
    wxTreeCtrl* m_pTree_ShaderGroups;
    wxTreeCtrl* m_pTree_SoundCues;
    wxTreeCtrl* m_pTree_Files;
    wxTreeCtrl* m_pTree_Buffers;
    wxTreeCtrl* m_pTree_DrawCalls;

    bool m_DrawCallListDirty;
    int m_DrawCallIndexToDraw;

protected:
    wxTreeItemId FindObject(wxTreeCtrl* tree, void* pObjectPtr, wxTreeItemId idroot);
    void UpdateRootNodeMaterialCount();
    void UpdateRootNodeTextureCount();
    void UpdateRootNodeShaderGroupCount();
    void UpdateRootNodeSoundCueCount();
    void UpdateRootNodeFileCount();
    void UpdateRootNodeBufferCount();
    void UpdateRootNodeDrawCallCount();

    void OnTabSelected(wxNotebookEvent& event);
    void OnTreeSelectionChanged(wxTreeEvent& event);
    void OnTreeContextMenuRequested(wxTreeEvent& event);
    void OnDragBegin(wxTreeEvent& event);

public:
    PanelMemory(wxFrame* parentframe);
    ~PanelMemory();

    //void Refresh();

    wxTreeItemId FindMaterialCategory(const char* category);
    void AddMaterial(MaterialDefinition* pMaterial, const char* category, const char* desc, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, PanelObjectListCallback pDragFunction);
    void RemoveMaterial(MaterialDefinition* pMaterial);
    void SetMaterialPanelCallbacks(wxTreeItemId treeid, void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, PanelObjectListCallback pDragFunction);
    const char* GetSelectedMaterialTreeItemText();
    MaterialDefinition* GetSelectedMaterial();

    void AddTexture(TextureDefinition* pTextureDef, const char* category, const char* desc, PanelObjectListCallback pDragFunction);
    void RemoveTexture(TextureDefinition* pTextureDef);

    void AddShaderGroup(ShaderGroup* pShaderGroup, const char* category, const char* desc, PanelObjectListCallback pDragFunction);
    void RemoveShaderGroup(ShaderGroup* pShaderGroup);

    void AddSoundCue(SoundCue* pSoundCue, const char* category, const char* desc, PanelObjectListCallback pDragFunction);
    void RemoveSoundCue(SoundCue* pSoundCue);
    void AddSoundObject(SoundObject* pSound, SoundCue* pSoundCue, const char* desc, PanelObjectListCallback pDragFunction);
    void RemoveSoundObject(SoundObject* pSound);
    void SetSoundPanelCallbacks(wxTreeItemId treeid, void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, PanelObjectListCallback pDragFunction);

    wxTreeItemId FindFile(MyFileObject* pFile);
    void AddFile(MyFileObject* pFile, const char* category, const char* desc, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, PanelObjectListCallback pDragFunction);
    void RemoveFile(MyFileObject* pFile);
    void SetFilePanelCallbacks(wxTreeItemId treeid, void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, PanelObjectListCallback pDragFunction);

    void AddBuffer(BufferDefinition* pBufferDef, const char* category, const char* desc);
    void UpdateBuffer(BufferDefinition* pBufferDef);
    void RemoveBuffer(BufferDefinition* pBufferDef);

    void AddDrawCall(int index, const char* category, const char* desc);
    void RemoveAllDrawCalls();
    void RemoveDrawCall(int index);

    void SetLabelEditFunction(wxTreeCtrl* pTree, void* pObject, PanelObjectListLabelEditCallback pLabelEditFunction);
    void OnTreeBeginLabelEdit(wxTreeEvent& event);
    void OnTreeEndLabelEdit(wxTreeEvent& event);
    wxString GetObjectName(wxTreeCtrl* pTree, void* pObject);
    void RenameObject(wxTreeCtrl* pTree, void* pObject, const char* desc);
};

#endif // __PanelMemory_H__
