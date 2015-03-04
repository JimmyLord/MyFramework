//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __PanelMemory_H__
#define __PanelMemory_H__

class PanelMemory;

extern PanelMemory* g_pPanelMemory;

class PanelMemory : public wxPanel
{
public:
    wxNotebook* m_pNotebook;
    wxTreeCtrl* m_pTree_Buffers;
    wxTreeCtrl* m_pTree_Textures;
    wxTreeCtrl* m_pTree_Files;
    wxTreeCtrl* m_pTree_DrawCalls;
    wxTreeCtrl* m_pTree_ShaderGroups;

    bool m_DrawCallListDirty;
    int m_DrawCallIndexToDraw;

protected:
    wxTreeItemId FindObject(wxTreeCtrl* tree, void* pObjectPtr, wxTreeItemId idroot);
    void UpdateRootNodeBufferCount();
    void UpdateRootNodeTextureCount();
    void UpdateRootNodeFileCount();
    void UpdateRootNodeDrawCallCount();
    void UpdateRootNodeShaderGroupCount();

    void OnDrawCallTabSelected(wxNotebookEvent& event);
    void OnDrawCallTreeSelectionChanged(wxTreeEvent& event);
    void OnDragBegin(wxTreeEvent& event);

public:
    PanelMemory(wxFrame* parentframe);
    ~PanelMemory();

    //void Refresh();
    void AddBuffer(BufferDefinition* pBufferDef, const char* category, const char* desc);
    void UpdateBuffer(BufferDefinition* pBufferDef);
    void RemoveBuffer(BufferDefinition* pBufferDef);

    void AddTexture(TextureDefinition* pTextureDef, const char* category, const char* desc, PanelObjectListCallback pDragFunction);
    void RemoveTexture(TextureDefinition* pTextureDef);

    void AddFile(MyFileObject* pFile, const char* category, const char* desc, PanelObjectListCallback pLeftClickFunction, PanelObjectListCallback pDragFunction);
    void RemoveFile(MyFileObject* pFile);

    void AddDrawCall(int index, const char* category, const char* desc);
    void RemoveAllDrawCalls();
    void RemoveDrawCall(int index);

    void AddShaderGroup(ShaderGroup* pShaderGroup, const char* category, const char* desc, PanelObjectListCallback pDragFunction);
    void RemoveShaderGroup(ShaderGroup* pShaderGroup);
};

#endif // __PanelMemory_H__
