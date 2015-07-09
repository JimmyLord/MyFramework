//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __PanelObjectList_H__
#define __PanelObjectList_H__

class PanelObjectList;

extern PanelObjectList* g_pPanelObjectList;

typedef void (*PanelObjectListCallback)(void*);
typedef void (*PanelObjectListCallbackLeftClick)(void*, wxTreeItemId id, unsigned int count);
typedef void (*PanelObjectListCallbackRightClick)(void*, wxTreeItemId id);
typedef void (*PanelObjectListCallbackDropTarget)(void* pObjectPtr, wxTreeItemId id, int controlid, wxCoord x, wxCoord y);
typedef void (*PanelObjectListLabelEditCallback)(void* pObjectPtr, wxTreeItemId id, wxString);

// Macros to make life easier if I change the params of any of these functions.
#define MYFW_PANELOBJECTLIST_DEFINE_CALLBACK_ONDROP(FunctionName, ObjectClass) \
    static void Static##FunctionName(void* pObjectPtr, wxTreeItemId id, int controlid, wxCoord x, wxCoord y) \
    { ((ObjectClass*)pObjectPtr)->FunctionName(id, controlid, x, y); } \
    void FunctionName(wxTreeItemId id, int controlid, wxCoord x, wxCoord y);

#define MYFW_PANELOBJECTLIST_DECLARE_CALLBACK_ONDROP(FunctionName, ObjectClass) \
    void ObjectClass::FunctionName(wxTreeItemId id, int controlid, wxCoord x, wxCoord y)

class TreeItemDataGenericObjectInfo : public wxTreeItemData
{
public:
    TreeItemDataGenericObjectInfo()
    {
        m_pObject = 0;

        m_pLeftClickFunction = 0;
        m_pRightClickFunction = 0;
        m_pDragFunction = 0;
        m_pDropFunction = 0;
        m_pLabelEditFunction = 0;

        m_pObject_LeftClick = 0;
        m_pObject_RightClick = 0;
        m_pObject_Drag = 0;
        m_pObject_Drop = 0;
        m_pObject_LabelEdit = 0;
    }

    void* m_pObject;
    PanelObjectListCallbackLeftClick m_pLeftClickFunction;
    PanelObjectListCallbackRightClick m_pRightClickFunction;
    PanelObjectListCallback m_pDragFunction;
    PanelObjectListCallbackDropTarget m_pDropFunction;
    PanelObjectListLabelEditCallback m_pLabelEditFunction;

    // a different object can receive the callback function if needed.
    void* m_pObject_LeftClick;
    void* m_pObject_RightClick;
    void* m_pObject_Drag;
    void* m_pObject_Drop;
    void* m_pObject_LabelEdit;
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

void UpdatePanelWatchWithSelectedItems();

class PanelObjectList : public wxPanel
{
public:
    wxTreeCtrl* m_pTree_Objects;
    bool m_UpdatePanelWatchOnSelection;

    void* m_pCallbackFunctionObject;
    PanelObjectListCallback m_pOnTreeSelectionChangedFunction;

    // Function panel watch will call if it needs a refresh.
    //static void StaticUpdatePanelWatchWithSelectedItems(void* pObjectPtr) { ((PanelObjectList*)pObjectPtr)->UpdatePanelWatchWithSelectedItems(); }
    //void UpdatePanelWatchWithSelectedItems();
    static void StaticUpdatePanelWatchWithSelectedItems(void* pObjectPtr) { UpdatePanelWatchWithSelectedItems(); }

    wxTreeItemId m_ItemSelectedBeforeDrag;

protected:
    wxTreeItemId FindObject(wxTreeCtrl* tree, void* pObject, wxTreeItemId idroot);
    void OnTreeItemLeftDown(wxMouseEvent& event);
    void OnTreeItemLeftUp(wxMouseEvent& event);
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
    void SetTreeRootData(void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction);
    wxTreeItemId AddObject(void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, const char* category, const char* desc);
    wxTreeItemId AddObject(void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, wxTreeItemId parentid, const char* desc);

    void SetDragAndDropFunctions(wxTreeItemId id, PanelObjectListCallback pDragFunction, PanelObjectListCallbackDropTarget pDropFunction);
    void SetLabelEditFunction(wxTreeItemId id, PanelObjectListLabelEditCallback pLabelEditFunction);
    void SetCustomObjectForCallback_LeftClick(wxTreeItemId id, void* pObject);
    void SetCustomObjectForCallback_RightClick(wxTreeItemId id, void* pObject);
    void SetCustomObjectForCallback_Drag(wxTreeItemId id, void* pObject);
    void SetCustomObjectForCallback_Drop(wxTreeItemId id, void* pObject);
    void SetCustomObjectForCallback_LabelEdit(wxTreeItemId id, void* pObject);
    
    void RemoveObject(void* pObject);
    void* GetObject(wxTreeItemId id);

    void Tree_MoveObject(void* pParent, void* pObjectToMove, void* pPreviousObject);

    wxTreeItemId FindObject(void* pObject);
    void SelectObject(void* pObject); // pass in 0 to unselect all.
    void UnselectObject(void* pObject);
    //void* GetSelectedObject(); // not useful, since there's no indication of object type.
    bool IsObjectSelected(void* pObject);
    wxString GetObjectName(void* pObject);
    void RenameObject(wxTreeItemId id, const char* desc);
    void RenameObject(void* pObject, const char* desc);
};

#endif // __PanelObjectList_H__
