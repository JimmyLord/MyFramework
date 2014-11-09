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

#include "CommonHeader.h"
#include "PanelObjectList.h"

PanelObjectList* g_pPanelObjectList = 0;

class TreeItemDataGenericObjectInfo : public wxTreeItemData
{
public:
    void* m_ObjectPtr;
    PanelObjectListCallback m_FunctionPtr;
}; 

PanelObjectList::PanelObjectList(wxFrame* parentframe)
: wxPanel( parentframe, wxID_ANY, wxDefaultPosition, wxSize(300, 600), wxTAB_TRAVERSAL | wxNO_BORDER, "Memory" )
{
    m_pTree_Objects = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxSize(2000,2000) );
    wxTreeItemId idroot = m_pTree_Objects->AddRoot( "Objects" );

    // setup a sizer to resize the notebook
    wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add( m_pTree_Objects, 0, wxGROW|wxALL, 2 );
    SetSizer( sizer );

    Update();

    Connect( wxEVT_TREE_SEL_CHANGED, wxTreeEventHandler(PanelObjectList::OnTreeSelectionChanged) );
}

PanelObjectList::~PanelObjectList()
{
    SAFE_DELETE( m_pTree_Objects );
}

void PanelObjectList::OnTreeSelectionChanged(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Objects->GetItemData( id );
    g_pPanelWatch->ClearAllVariables();
    if( pData )
    {
        (pData->m_FunctionPtr)(pData->m_ObjectPtr);
    }
}

wxTreeItemId PanelObjectList::FindObject(wxTreeCtrl* tree, void* pObjectPtr, wxTreeItemId idroot)
{
    wxTreeItemIdValue cookie;
    wxTreeItemId id = tree->GetFirstChild( idroot, cookie );
    while( id.IsOk() )
    {
        wxTreeItemData* pData = tree->GetItemData( id );
        if( pData )
        {
            void* objptr = ((TreeItemDataGenericObjectInfo*)pData)->m_ObjectPtr;
            if( objptr == pObjectPtr )
            {
                return id;
            }
        }

        if( tree->ItemHasChildren( id ) )
        {
            id = FindObject( tree, pObjectPtr, id );
            if( id.IsOk() )
                return id;
        }

        id = tree->GetNextChild( idroot, cookie );
    }

    return wxTreeItemId();
}

wxTreeItemId PanelObjectList::FindObject(void* pObjectPtr)
{
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Objects, pObjectPtr, idroot );

    return id;
}

void PanelObjectList::UpdateRootNodeObjectCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    // update root node object count.
    sprintf_s( tempstr, 100, "Objects(%d)",
        m_pTree_Objects->GetChildrenCount( idroot, true ) - m_pTree_Objects->GetChildrenCount( idroot, false ) );
    m_pTree_Objects->SetItemText( idroot, tempstr );
}

//void PanelObjectList::Refresh()
//{
//}

wxTreeItemId PanelObjectList::GetTreeRoot()
{
    return m_pTree_Objects->GetRootItem();
}

wxTreeItemId PanelObjectList::AddObject(void* pObjectPtr, PanelObjectListCallback pFunctionPtr, const char* category, const char* desc)
{
    assert( pObjectPtr != 0 );

    char tempstr[100];

    wxTreeItemId newid;

    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();
    int count = m_pTree_Objects->GetChildrenCount( idroot, false );

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

    int categorycount = m_pTree_Objects->GetChildrenCount( idcategory );

    // insert the Texture into it's category
    {
        //sprintf_s( tempstr, 100, "%s %d", desc, categorycount );
        sprintf_s( tempstr, 100, "%s", desc );
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_ObjectPtr = pObjectPtr;
        pData->m_FunctionPtr = pFunctionPtr;

        newid = m_pTree_Objects->AppendItem( idcategory, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
        {
            m_pTree_Objects->Expand( idroot );
        }
    }

    UpdateRootNodeObjectCount();

    return newid;
}

wxTreeItemId PanelObjectList::AddObject(void* pObjectPtr, PanelObjectListCallback pFunctionPtr, wxTreeItemId parentid, const char* desc)
{
    assert( pObjectPtr != 0 );

    //char tempstr[100];

    wxTreeItemId newid;

    // get the root count before adding the item for check below.
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();
    int count = m_pTree_Objects->GetChildrenCount( idroot, false );

    // insert the Object under it's parent node
    {
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_ObjectPtr = pObjectPtr;
        pData->m_FunctionPtr = pFunctionPtr;

        newid = m_pTree_Objects->AppendItem( parentid, desc, -1, -1, pData );
        assert( newid.IsOk() );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
        {
            m_pTree_Objects->Expand( idroot );
        }
    }

    UpdateRootNodeObjectCount();

    return newid;
}

void PanelObjectList::RemoveObject(void* pObjectPtr)
{
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Objects, pObjectPtr, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        wxTreeItemId parentid = m_pTree_Objects->GetItemParent( id );

        m_pTree_Objects->Delete( id );

        if( m_pTree_Objects->GetChildrenCount( parentid ) == 0 )
            m_pTree_Objects->Delete( parentid );
    }

    UpdateRootNodeObjectCount();
}

void PanelObjectList::RenameObject(void* pObjectPtr, const char* desc)
{
    wxTreeItemId idroot = m_pTree_Objects->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Objects, pObjectPtr, idroot );

    if( id.IsOk() )
    {
        m_pTree_Objects->SetItemText( id, desc );
    }

    UpdateRootNodeObjectCount();
}
