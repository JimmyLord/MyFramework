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

#ifndef __PanelObjectList_H__
#define __PanelObjectList_H__

class PanelObjectList;

extern PanelObjectList* g_pPanelObjectList;

typedef void (*PanelObjectListCallback)(void*);

class TreeItemDataGenericObjectInfo : public wxTreeItemData
{
public:
    TreeItemDataGenericObjectInfo()
    {
        m_pLeftClickFunction = 0;
        m_pRightClickFunction = 0;
        m_pLabelEditFunction = 0;
        m_pDragFunction = 0;
        m_pDropFunction = 0;
    }

    void* m_pObject;
    PanelObjectListCallback m_pLeftClickFunction;
    PanelObjectListCallback m_pRightClickFunction;
    PanelObjectListCallback m_pLabelEditFunction;
    PanelObjectListCallback m_pDragFunction;
    PanelObjectListCallback m_pDropFunction;
}; 

class PanelObjectListDropTarget : public wxDropTarget
{
public:
    PanelObjectList* m_pPanelObjectList;

public:
    PanelObjectListDropTarget();

    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);
    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult);
};

class PanelObjectList : public wxPanel
{
public:
    wxTreeCtrl* m_pTree_Objects;

    void* m_pCallbackFunctionObject;
    PanelObjectListCallback m_pOnTreeSelectionChangedFunction;

protected:
    wxTreeItemId FindObject(wxTreeCtrl* tree, void* pObject, wxTreeItemId idroot);
    void OnTreeSelectionChanged(wxTreeEvent& event);
    void OnTreeBeginLabelEdit(wxTreeEvent& event);
    void OnTreeEndLabelEdit(wxTreeEvent& event);
    void OnTreeContextMenuRequested(wxTreeEvent& event);
    void OnDragBegin(wxTreeEvent& event);
    void UpdateRootNodeObjectCount();

    // drop code for drag and drop.
//    virtual wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult defResult);
//    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);
//    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult);

public:
    PanelObjectList(wxFrame* parentframe);
    ~PanelObjectList();

    //void Refresh();
    wxTreeItemId GetTreeRoot();
    void SetTreeRootData(void* pObject, PanelObjectListCallback pLeftClickFunction, PanelObjectListCallback pRightClickFunction);
    wxTreeItemId AddObject(void* pObject, PanelObjectListCallback pLeftClickFunction, PanelObjectListCallback pRightClickFunction, const char* category, const char* desc);
    wxTreeItemId AddObject(void* pObject, PanelObjectListCallback pLeftClickFunction, PanelObjectListCallback pRightClickFunction, wxTreeItemId parentid, const char* desc);
    void SetDragAndDropFunctions(void* pObject, PanelObjectListCallback pDragFunction, PanelObjectListCallback pDropFunction);
    void SetLabelEditFunction(void* pObject, PanelObjectListCallback pLabelEditFunction);
    void RemoveObject(void* pObject);
    void* GetObject(wxTreeItemId id);

    wxTreeItemId FindObject(void* pObject);
    void SelectObject(void* pObject); // pass in 0 to unselect all.
    //void* GetSelectedObject(); // not useful, since there's no indication of object type.
    wxString GetObjectName(void* pObject);
    void RenameObject(void* pObject, const char* desc);
};

#endif // __PanelObjectList_H__
