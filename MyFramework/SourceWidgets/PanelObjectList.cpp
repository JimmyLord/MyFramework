//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "PanelObjectList.h"

PanelObjectList* g_pPanelObjectList = 0;

PanelObjectList::PanelObjectList(wxFrame* parentframe)
: wxPanel( parentframe, wxID_ANY, wxDefaultPosition, wxSize(300, 600), wxTAB_TRAVERSAL | wxNO_BORDER, "Memory" )
{
    m_pTree_Objects = MyNew wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxSize(2000,2000),
        wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_EDIT_LABELS | wxTR_MULTIPLE );
    //wxTreeItemId idroot =
    m_pTree_Objects->AddRoot( "Objects" );

    // setup a sizer to resize the tree
    wxBoxSizer* sizer = MyNew wxBoxSizer( wxHORIZONTAL );
    sizer->Add( m_pTree_Objects, 0, wxGROW|wxALL, 2 );
    SetSizer( sizer );

    PanelObjectListDropTarget* pDropTarget = MyNew PanelObjectListDropTarget;
    pDropTarget->m_pPanelObjectList = this;
    m_pTree_Objects->SetDropTarget( pDropTarget );

    m_pCallbackFunctionObject = 0;
    m_pOnTreeSelectionChangedFunction = 0;
    m_UpdatePanelWatchOnSelection = true;

    Update();

    Connect( wxEVT_TREE_SEL_CHANGED, wxTreeEventHandler( PanelObjectList::OnTreeSelectionChanged ) );
    m_pTree_Objects->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( PanelObjectList::OnTreeItemLeftDown ) );
    //Connect( wxEVT_LEFT_UP, wxMouseEventHandler( PanelObjectList::OnTreeItemLeftUp ) );
    //m_pTree_Objects->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( PanelObjectList::OnTreeItemLeftUp ) );
    Connect( wxEVT_TREE_BEGIN_LABEL_EDIT, wxTreeEventHandler( PanelObjectList::OnTreeBeginLabelEdit ) );
    Connect( wxEVT_TREE_END_LABEL_EDIT, wxTreeEventHandler( PanelObjectList::OnTreeEndLabelEdit ) );
    Connect( wxEVT_TREE_ITEM_MENU, wxTreeEventHandler( PanelObjectList::OnTreeContextMenuRequested ) );
    Connect( wxEVT_TREE_BEGIN_DRAG, wxTreeEventHandler( PanelObjectList::OnDragBegin ) );
}

PanelObjectList::~PanelObjectList()
{
    SAFE_DELETE( m_pTree_Objects );
}

void PanelObjectList::OnTreeItemLeftDown(wxMouseEvent& event)
{
    // store the first selected item, for cases when the new selected item is dragged.
    // TODO: DRAGOBJECTLISTITEM: find a better way to deal with this.
    wxArrayTreeItemIds selecteditems;
    unsigned int numselected = (unsigned int)g_pPanelObjectList->m_pTree_Objects->GetSelections( selecteditems );
    
    if( numselected > 0 )
    {
        g_pPanelObjectList->m_ItemSelectedBeforeDrag = selecteditems[0].GetID();
        //LOGInfo( LOGTag, "PanelObjectList::OnTreeItemLeftDown\n" );
    }

    event.Skip();
}

//void PanelObjectList::OnTreeItemLeftUp(wxMouseEvent& event)
//{
//    UpdatePanelWatchWithSelectedItems();
//    event.Skip();
//}

void PanelObjectList::OnTreeSelectionChanged(wxTreeEvent& event)
{
    if( m_UpdatePanelWatchOnSelection )
        UpdatePanelWatchWithSelectedItems();
}

void UpdatePanelWatchWithSelectedItems()
{
    g_pPanelWatch->SetRefreshCallback( g_pPanelObjectList, PanelObjectList::StaticUpdatePanelWatchWithSelectedItems );

    // stop draws on watch panel
    g_pPanelWatch->Freeze();

    //LOGInfo( LOGTag, "PanelObjectList::OnTreeSelectionChanged\n" );

    wxArrayTreeItemIds selecteditems;
    unsigned int numselected = (unsigned int)g_pPanelObjectList->m_pTree_Objects->GetSelections( selecteditems );

    if( g_pPanelObjectList->m_pOnTreeSelectionChangedFunction )
    {
        g_pPanelObjectList->m_pOnTreeSelectionChangedFunction( g_pPanelObjectList->m_pCallbackFunctionObject );
    }

	// TODO: if multiple selected, show common properties.
    //unsigned int i = 0;
    //if( numselected > 0 )
    for( unsigned int i=0; i<numselected; i++ )
    {
        // pass left click event through to the item.
        wxTreeItemId id = selecteditems[i].GetID();  //event.GetItem();
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)g_pPanelObjectList->m_pTree_Objects->GetItemData( id );
        //g_pPanelWatch->ClearAllVariables(); // should be done by item itself, in case it doesn't want to update watch window.

        if( pData && pData->m_pLeftClickFunction )
        {
            pData->m_pLeftClickFunction( pData->m_pObject, id, i );
        }
    }

    // resume draws on watch panel
    g_pPanelWatch->UpdatePanel();
    g_pPanelWatch->Thaw();
}

void PanelObjectList::OnTreeBeginLabelEdit(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Objects->GetItemData( id );

    // don't allow edits if there's no callback.
    if( pData == 0 || pData->m_pLabelEditFunction == 0 )
    {
        // cancel the edit
        event.Veto();
    }
}

void PanelObjectList::OnTreeEndLabelEdit(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Objects->GetItemData( id );

    if( pData && pData->m_pLabelEditFunction )
    {
        //// end the edit to commit the new name
        //m_pTree_Objects->EndEditLabel( id );

        wxString newlabel = event.GetLabel();

        if( event.IsEditCancelled() == false )
        {
            // manually set the item to the new name, so the callback func can query for the name.
            //    the edit process seems to happen later
            RenameObject( pData->m_pObject, newlabel );

            // Call the callback and let game code handle the new name
            pData->m_pLabelEditFunction( pData->m_pObject, id, newlabel );
        }
    }
}

void PanelObjectList::OnTreeContextMenuRequested(wxTreeEvent& event)
{
    // pass right click events through to the item.
    wxTreeItemId id = event.GetItem();
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Objects->GetItemData( id );
    if( pData && pData->m_pRightClickFunction )
    {
        pData->m_pRightClickFunction( pData->m_pObject, id );
    }
}

void PanelObjectList::OnDragBegin(wxTreeEvent& event)
{
    //LOGInfo( LOGTag, "PanelObjectList::OnDragBegin\n" );

    // let the object know its being dragged, so it can store it's data.
    // This only works within this app, not between apps.

    // get the pointer to the tree affected.
    wxTreeCtrl* pTree = m_pTree_Objects;

    // reselect the item that was selected before dragging
    // TODO: DRAGOBJECTLISTITEM: find a better way to deal with this.
    if( m_ItemSelectedBeforeDrag.IsOk() )
    {
        m_pTree_Objects->UnselectAll();
        m_pTree_Objects->SelectItem( m_ItemSelectedBeforeDrag );
    }

    wxTreeItemId id = event.GetItem();
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)pTree->GetItemData( id );
    if( pData && pData->m_pDragFunction )
    {
        pData->m_pDragFunction( pData->m_pObject );
    }
    else
    {
        return; // cancel drag and drop.
    }

    // dummy data to kick off the drag/drop op.  Real data is handled by objects in list.
#if MYFW_WINDOWS
    wxCustomDataObject dataobject;
    dataobject.SetFormat( *g_pMyDataFormat );
    wxDropSource dragsource( dataobject );    
    wxDragResult result = dragsource.DoDragDrop( wxDrag_CopyOnly );
#endif
}

PanelObjectListDropTarget::PanelObjectListDropTarget()
{
    wxCustomDataObject* dataobject = MyNew wxCustomDataObject;
    dataobject->SetFormat( *g_pMyDataFormat );
    SetDataObject( dataobject );
}

wxDragResult PanelObjectListDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
    return wxDragCopy;
}

wxDragResult PanelObjectListDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult defResult)
{
    // figure out which object the stuff was dropped on and let it know.
    wxTreeItemId id = m_pPanelObjectList->m_pTree_Objects->HitTest( wxPoint(x, y) );
    if( id.IsOk() )
    {
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pPanelObjectList->m_pTree_Objects->GetItemData( id );
        if( pData && pData->m_pDropFunction )
        {
            pData->m_pDropFunction( pData->m_pObject, id, -1, x, y );
        }
    }

    return wxDragNone;
}

wxTreeItemId PanelObjectList::FindObject(wxTreeCtrl* tree, void* pObject, wxTreeItemId idroot)
{
    wxTreeItemIdValue cookie;
    wxTreeItemId id = tree->GetFirstChild( idroot, cookie );
    while( id.IsOk() )
    {
        wxTreeItemData* pData = tree->GetItemData( id );
        if( pData )
        {
            void* objptr = ((TreeItemDataGenericObjectInfo*)pData)->m_pObject;
            if( objptr == pObject )
            {
                return id;
            }
        }

        if( tree->ItemHasChildren( id ) )
        {
            id = FindObject( tree, pObject, id );
            if( id.IsOk() )
                return id;
        }

        id = tree->GetNextChild( idroot, cookie );
    }

    return wxTreeItemId();
}

wxTreeItemId PanelObjectList::FindObject(void* pObject)
{
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Objects, pObject, idroot );

    return id;
}

void PanelObjectList::SelectObject(void* pObject)
{
    if( pObject == 0 )
    {
        m_pTree_Objects->UnselectAll();
        return;
    }

    wxTreeItemId id = FindObject( pObject );

    if( id.IsOk() )
    {
        m_pTree_Objects->SelectItem( id );
    }
}

void PanelObjectList::UnselectObject(void* pObject)
{
    wxTreeItemId id = FindObject( pObject );

    if( id.IsOk() )
    {
        m_pTree_Objects->UnselectItem( id );
    }
}

// not useful, since there's no indication of object type.
//void* PanelObjectList::GetSelectedObject()
//{
//    wxTreeItemId id = m_pTree_Objects->GetSelection();
//
//    if( id.IsOk() )
//    {
//        wxTreeItemData* pData = m_pTree_Objects->GetItemData( id );
//        if( pData )
//        {
//            void* objptr = ((TreeItemDataGenericObjectInfo*)pData)->m_pObject;
//            return objptr;
//        }
//    }
//
//    return 0;
//}

void PanelObjectList::UpdateRootNodeObjectCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    if( idroot.IsOk() )
    {
        // update root node object count.
        sprintf_s( tempstr, 100, "Objects(%d)",
            (int)m_pTree_Objects->GetChildrenCount( idroot, true ) - (int)m_pTree_Objects->GetChildrenCount( idroot, false ) );
        m_pTree_Objects->SetItemText( idroot, tempstr );
    }
}

//void PanelObjectList::Refresh()
//{
//}

wxTreeItemId PanelObjectList::GetTreeRoot()
{
    return m_pTree_Objects->GetRootItem();
}

void PanelObjectList::SetTreeRootData(void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction)
{
    MyAssert( pObject != 0 );

    // get the root count before adding the item for check below.
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    // insert the Object under it's parent node
    {
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pObject;
        pData->m_pLeftClickFunction = pLeftClickFunction;
        pData->m_pRightClickFunction = pRightClickFunction;

        m_pTree_Objects->SetItemData( idroot, pData );
    }
}

wxTreeItemId PanelObjectList::AddObject(void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, const char* category, const char* desc)
{
    MyAssert( pObject != 0 );

    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();
    //int count = (int)m_pTree_Objects->GetChildrenCount( idroot, false );

    // see if the category exists
    wxTreeItemId idcategory;
    {
        wxTreeItemIdValue cookie;
        idcategory = m_pTree_Objects->GetFirstChild( idroot, cookie );
        while( idcategory.IsOk() )
        {
            wxString catstr = m_pTree_Objects->GetItemText( idcategory );
            if( catstr == category )
                break;

            idcategory = m_pTree_Objects->GetNextChild( idroot, cookie );
        }
    }
    
    // insert the category if necessary
    if( idcategory.IsOk() == false )
    {
        idcategory = m_pTree_Objects->AppendItem( idroot, category, -1, -1, 0 );
    }

    return AddObject( pObject, pLeftClickFunction, pRightClickFunction, idcategory, desc );
}

wxTreeItemId PanelObjectList::AddObject(void* pObject, PanelObjectListCallbackLeftClick pLeftClickFunction, PanelObjectListCallbackRightClick pRightClickFunction, wxTreeItemId parentid, const char* desc)
{
    MyAssert( pObject != 0 );

    wxTreeItemId newid;

    // get the root count before adding the item for check below.
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();
    int count = (int)m_pTree_Objects->GetChildrenCount( idroot, false );

    // insert the Object under it's parent node
    {
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pObject;
        pData->m_pLeftClickFunction = pLeftClickFunction;
        pData->m_pRightClickFunction = pRightClickFunction;

        newid = m_pTree_Objects->AppendItem( parentid, desc, -1, -1, pData );
        MyAssert( newid.IsOk() );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
        {
            m_pTree_Objects->Expand( idroot );
        }
    }

    UpdateRootNodeObjectCount();

    return newid;
}

void PanelObjectList::SetDragAndDropFunctions(wxTreeItemId id, PanelObjectListCallback pDragFunction, PanelObjectListCallbackDropTarget pDropFunction)
{
    //wxTreeItemId idroot = m_pTree_Objects->GetRootItem();
    //wxTreeItemId id = FindObject( m_pTree_Objects, pObject, idroot );
    if( id.IsOk() )
    {
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Objects->GetItemData( id );
        pData->m_pDragFunction = pDragFunction;
        pData->m_pDropFunction = pDropFunction;
    }
}

void PanelObjectList::SetLabelEditFunction(wxTreeItemId id, PanelObjectListLabelEditCallback pLabelEditFunction)
{
    //wxTreeItemId idroot = m_pTree_Objects->GetRootItem();
    //wxTreeItemId id = FindObject( m_pTree_Objects, pObject, idroot );
    if( id.IsOk() )
    {
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Objects->GetItemData( id );
        pData->m_pLabelEditFunction = pLabelEditFunction;
    }
}

void PanelObjectList::RemoveObject(void* pObject)
{
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Objects, pObject, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        //wxTreeItemId parentid = m_pTree_Objects->GetItemParent( id );

        m_pTree_Objects->Delete( id );

        //if( m_pTree_Objects->GetChildrenCount( parentid ) == 0 && parentid != rootid )
        //    m_pTree_Objects->Delete( parentid );
    }

    UpdateRootNodeObjectCount();
}

void* PanelObjectList::GetObject(wxTreeItemId id)
{
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Objects->GetItemData( id );

    return pData->m_pObject;
}

wxString PanelObjectList::GetObjectName(void* pObject)
{
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Objects, pObject, idroot );

    if( id.IsOk() )
    {
        return m_pTree_Objects->GetItemText( id );
    }

    return "";
}

void PanelObjectList::RenameObject(void* pObject, const char* desc)
{
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Objects, pObject, idroot );

    if( id.IsOk() )
    {
        m_pTree_Objects->SetItemText( id, desc );
    }

    UpdateRootNodeObjectCount();
}
