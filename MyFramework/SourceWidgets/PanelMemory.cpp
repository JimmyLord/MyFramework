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
#include "PanelMemory.h"

PanelMemory* g_pPanelMemory = 0;

class TreeItemDataVoidPtr : public wxTreeItemData
{
public:
    void* m_ObjectPtr;
};

PanelMemory::PanelMemory(wxFrame* parentframe)
: wxPanel( parentframe, wxID_ANY, wxDefaultPosition, wxSize(300, 600), wxTAB_TRAVERSAL | wxNO_BORDER, "Memory" )
{
    wxTreeItemId idroot;

    // create a notebook with 3 pages(buffers/textures/files)
    m_pNotebook = MyNew wxNotebook( this, wxID_ANY, wxPoint(0,0), wxSize(2000,2000) );

    m_pTree_Buffers = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxSize(2000,2000) );
    idroot = m_pTree_Buffers->AddRoot( "Buffers" );
    m_pNotebook->AddPage( m_pTree_Buffers, "Buffers" );

    m_pTree_Textures = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxSize(2000,2000) );
    idroot = m_pTree_Textures->AddRoot( "Textures" );
    m_pNotebook->AddPage( m_pTree_Textures, "Textures" );

    m_pTree_Files = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxSize(2000,2000) );
    idroot = m_pTree_Files->AddRoot( "Files" );
    m_pNotebook->AddPage( m_pTree_Files, "Files" );

    m_pTree_DrawCalls = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxSize(2000,2000) );
    idroot = m_pTree_DrawCalls->AddRoot( "Draws" );
    m_pNotebook->AddPage( m_pTree_DrawCalls, "Draws" );

    // setup a sizer to resize the notebook
    wxBoxSizer* sizer = MyNew wxBoxSizer( wxHORIZONTAL );
    sizer->Add( m_pNotebook, 0, wxGROW|wxALL, 2 );
    SetSizer( sizer );

    Update();

    Connect( wxEVT_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler(PanelMemory::OnDrawCallTabSelected) );
    Connect( wxEVT_TREE_SEL_CHANGED, wxTreeEventHandler(PanelMemory::OnDrawCallTreeSelectionChanged) );

    m_DrawCallListDirty = false;
    m_DrawCallIndexToDraw = -1;
}

PanelMemory::~PanelMemory()
{
    SAFE_DELETE( m_pTree_Buffers );
    SAFE_DELETE( m_pTree_Textures );
    SAFE_DELETE( m_pTree_Files );
    SAFE_DELETE( m_pTree_DrawCalls );
}

wxTreeItemId PanelMemory::FindObject(wxTreeCtrl* tree, void* pObjectPtr, wxTreeItemId idroot)
{
    wxTreeItemIdValue cookie;
    wxTreeItemId id = tree->GetFirstChild( idroot, cookie );
    while( id.IsOk() )
    {
        wxTreeItemData* pData = tree->GetItemData( id );
        if( pData )
        {
            void* objptr = ((TreeItemDataVoidPtr*)pData)->m_ObjectPtr;
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

//void PanelMemory::Refresh() // not storing category or desc... so can't really be refreshed.
//{
//    char tempstr[100];
//
//    if( g_pBufferManager == 0 )
//        return;
//
//    m_pTree_Buffers->DeleteAllItems();
//    wxTreeItemId idroot = m_pTree_Buffers->AddRoot( "Buffers" );
//    //wxTreeItemId idroot = m_pTree_Buffers->GetRootItem();
//
//    unsigned int count = 0;
//
//    for( CPPListNode* pNode = g_pBufferManager->m_Buffers.GetHead(); pNode; )
//    {
//        BufferDefinition* pBufferDef = (BufferDefinition*)pNode;
//        pNode = pNode->GetNext();
//
//        //wxTreeItemIdValue cookie;
//        //wxTreeItemId id = m_pTree_Buffers->GetFirstChild( idroot, cookie );
//        //while( id.IsOk() )
//        //{
//        //    wxTreeItemData* pData = m_pTree_Buffers->GetItemData( id );
//        //    void* objptr = ((TreeItemDataVoidPtr*)pData)->m_ptr;
//        //    if( objptr == pBufferDef )
//        //        break;
//
//        //    id = m_pTree_Buffers->GetNextChild( idroot, cookie );
//        //}
//
//        //if( id.IsOk() == false )
//        {
//            sprintf_s( tempstr, 100, "Buffer %d - size(%d) - num(%d)", count, pBufferDef->m_DataSize, pBufferDef->m_NumBuffersToUse );
//            TreeItemDataVoidPtr* pData = MyNew TreeItemDataVoidPtr();
//            pData->m_ptr = pBufferDef;
//
//            m_pTree_Buffers->InsertItem( idroot, count, tempstr, -1, -1, pData );
//        }
//
//        count++;
//    }
//
//    sprintf_s( tempstr, 100, "Buffers(%d) - size(%d)", m_pTree_Buffers->GetChildrenCount( idroot, true ),
//        g_pBufferManager->CalculateTotalMemoryUsedByBuffers() );
//    m_pTree_Buffers->SetItemText( idroot, tempstr );
//}

void PanelMemory::AddBuffer(BufferDefinition* pBufferDef, const char* category, const char* desc)
{
    assert( pBufferDef != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Buffers->GetRootItem();
    int count = m_pTree_Buffers->GetChildrenCount( idroot, false );

    // see if the category exists
    wxTreeItemId idcategory;
    {
        wxTreeItemIdValue cookie;
        idcategory = m_pTree_Buffers->GetFirstChild( idroot, cookie );
        while( idcategory.IsOk() )
        {
            wxString catstr = m_pTree_Buffers->GetItemText( idcategory );
            if( catstr == category )
                break;

            idcategory = m_pTree_Buffers->GetNextChild( idroot, cookie );
        }
    }
    
    // insert the category if necessary
    if( idcategory.IsOk() == false )
    {
        idcategory = m_pTree_Buffers->AppendItem( idroot, category, -1, -1, 0 );
    }

    int categorycount = m_pTree_Buffers->GetChildrenCount( idcategory );

    // insert the buffer into it's category
    {
        sprintf_s( tempstr, 100, "%s %d - size(%d) - num(%d)", desc, categorycount, pBufferDef->m_DataSize, pBufferDef->m_NumBuffersToUse );
        TreeItemDataVoidPtr* pData = MyNew TreeItemDataVoidPtr();
        pData->m_ObjectPtr = pBufferDef;

        m_pTree_Buffers->AppendItem( idcategory, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
            m_pTree_Buffers->Expand( idroot );
    }

    UpdateRootNodeBufferCount();
}

void PanelMemory::RemoveBuffer(BufferDefinition* pBufferDef)
{
    wxTreeItemId idroot = m_pTree_Buffers->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Buffers, pBufferDef, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        wxTreeItemId parentid = m_pTree_Buffers->GetItemParent( id );

        m_pTree_Buffers->Delete( id );

        if( m_pTree_Buffers->GetChildrenCount( parentid ) == 0 )
            m_pTree_Buffers->Delete( parentid );

        UpdateRootNodeBufferCount();
    }
}

void PanelMemory::UpdateRootNodeBufferCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Buffers->GetRootItem();

    // update root node memory usage count.
    sprintf_s( tempstr, 100, "Buffers - size(%d)", g_pBufferManager->CalculateTotalMemoryUsedByBuffers() );
    m_pTree_Buffers->SetItemText( idroot, tempstr );

    sprintf_s( tempstr, 100, "Buffers(%d)",
        m_pTree_Buffers->GetChildrenCount( idroot, true ) - m_pTree_Buffers->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( 0, tempstr );
}

void PanelMemory::AddTexture(TextureDefinition* pTextureDef, const char* category, const char* desc)
{
    assert( pTextureDef != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Textures->GetRootItem();
    int count = m_pTree_Textures->GetChildrenCount( idroot, false );

    // see if the category exists
    wxTreeItemId idcategory;
    {
        wxTreeItemIdValue cookie;
        idcategory = m_pTree_Textures->GetFirstChild( idroot, cookie );
        while( idcategory.IsOk() )
        {
            wxString catstr = m_pTree_Textures->GetItemText( idcategory );
            if( catstr == category )
                break;

            idcategory = m_pTree_Textures->GetNextChild( idroot, cookie );
        }
    }
    
    // insert the category if necessary
    if( idcategory.IsOk() == false )
    {
        idcategory = m_pTree_Textures->AppendItem( idroot, category, -1, -1, 0 );
    }

    int categorycount = m_pTree_Textures->GetChildrenCount( idcategory );

    // insert the Texture into it's category
    {
        sprintf_s( tempstr, 100, "%s %d - size(%d) - fileinmemory(%d)", desc, categorycount, pTextureDef->m_MemoryUsed, pTextureDef->m_pFile?1:0 );
        TreeItemDataVoidPtr* pData = MyNew TreeItemDataVoidPtr();
        pData->m_ObjectPtr = pTextureDef;

        m_pTree_Textures->AppendItem( idcategory, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
            m_pTree_Textures->Expand( idroot );
    }

    UpdateRootNodeTextureCount();
}

void PanelMemory::RemoveTexture(TextureDefinition* pTextureDef)
{
    wxTreeItemId idroot = m_pTree_Textures->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Textures, pTextureDef, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        wxTreeItemId parentid = m_pTree_Textures->GetItemParent( id );

        m_pTree_Textures->Delete( id );

        if( m_pTree_Textures->GetChildrenCount( parentid ) == 0 )
            m_pTree_Textures->Delete( parentid );

        UpdateRootNodeTextureCount();
    }
}

void PanelMemory::UpdateRootNodeTextureCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Textures->GetRootItem();

    // update root node memory usage count.
    //sprintf_s( tempstr, 100, "Textures(%d) - size(%d)",
    //    m_pTree_Textures->GetChildrenCount( idroot, true ) - m_pTree_Textures->GetChildrenCount( idroot, false ),
    //    0 );//g_pTextureManager->CalculateTotalMemoryUsedByTextures() );
    //m_pTree_Textures->SetItemText( idroot, tempstr );
    sprintf_s( tempstr, 100, "Textures(%d)",
        m_pTree_Textures->GetChildrenCount( idroot, true ) - m_pTree_Textures->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( 1, tempstr );
}

void PanelMemory::AddFile(MyFileObject* pFile, const char* category, const char* desc)
{
    assert( pFile != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Files->GetRootItem();
    int count = m_pTree_Files->GetChildrenCount( idroot, false );

    // see if the category exists
    wxTreeItemId idcategory;
    {
        wxTreeItemIdValue cookie;
        idcategory = m_pTree_Files->GetFirstChild( idroot, cookie );
        while( idcategory.IsOk() )
        {
            wxString catstr = m_pTree_Files->GetItemText( idcategory );
            if( catstr == category )
                break;

            idcategory = m_pTree_Files->GetNextChild( idroot, cookie );
        }
    }
    
    // insert the category if necessary
    if( idcategory.IsOk() == false )
    {
        idcategory = m_pTree_Files->AppendItem( idroot, category, -1, -1, 0 );
    }

    int categorycount = m_pTree_Files->GetChildrenCount( idcategory );

    // insert the File into it's category
    {
        sprintf_s( tempstr, 100, "%s %d - size(%d)", desc, categorycount, pFile->m_FileLength );
        TreeItemDataVoidPtr* pData = MyNew TreeItemDataVoidPtr();
        pData->m_ObjectPtr = pFile;

        m_pTree_Files->AppendItem( idcategory, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
            m_pTree_Files->Expand( idroot );
    }

    UpdateRootNodeFileCount();
}

void PanelMemory::RemoveFile(MyFileObject* pFile)
{
    wxTreeItemId idroot = m_pTree_Files->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Files, pFile, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        wxTreeItemId parentid = m_pTree_Files->GetItemParent( id );

        m_pTree_Files->Delete( id );

        if( m_pTree_Files->GetChildrenCount( parentid ) == 0 )
            m_pTree_Files->Delete( parentid );

        UpdateRootNodeFileCount();
    }
}

void PanelMemory::UpdateRootNodeFileCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Files->GetRootItem();

    // update root node memory usage count.
    sprintf_s( tempstr, 100, "Files - size(%d)", g_pFileManager->CalculateTotalMemoryUsedByFiles() );
    m_pTree_Files->SetItemText( idroot, tempstr );

    sprintf_s( tempstr, 100, "Files(%d)",
        m_pTree_Files->GetChildrenCount( idroot, true ) - m_pTree_Files->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( 2, tempstr );
}

void PanelMemory::AddDrawCall(int index, const char* category, const char* desc)
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_DrawCalls->GetRootItem();
    int count = m_pTree_DrawCalls->GetChildrenCount( idroot, false );

    // see if the category exists
    wxTreeItemId idcategory;
    {
        wxTreeItemIdValue cookie;
        idcategory = m_pTree_DrawCalls->GetFirstChild( idroot, cookie );
        while( idcategory.IsOk() )
        {
            wxString catstr = m_pTree_DrawCalls->GetItemText( idcategory );
            if( catstr == category )
                break;

            idcategory = m_pTree_DrawCalls->GetNextChild( idroot, cookie );
        }
    }
    
    // insert the category if necessary
    if( idcategory.IsOk() == false )
    {
        idcategory = m_pTree_DrawCalls->AppendItem( idroot, category, -1, -1, 0 );
    }

    int categorycount = m_pTree_DrawCalls->GetChildrenCount( idcategory );

    // insert the DrawCall into it's category
    {
        sprintf_s( tempstr, 100, "%s %d - size(%d)", desc, categorycount, 1 );
        TreeItemDataVoidPtr* pData = MyNew TreeItemDataVoidPtr();
        pData->m_ObjectPtr = (void*)index;

        m_pTree_DrawCalls->AppendItem( idcategory, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
            m_pTree_DrawCalls->Expand( idroot );
    }

    UpdateRootNodeDrawCallCount();
}

void PanelMemory::RemoveDrawCall(int index)
{
    wxTreeItemId idroot = m_pTree_DrawCalls->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_DrawCalls, (void*)index, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        wxTreeItemId parentid = m_pTree_DrawCalls->GetItemParent( id );

        m_pTree_DrawCalls->Delete( id );

        if( m_pTree_DrawCalls->GetChildrenCount( parentid ) == 0 )
            m_pTree_DrawCalls->Delete( parentid );

        UpdateRootNodeDrawCallCount();
    }
}


void PanelMemory::RemoveAllDrawCalls()
{
    wxTreeItemId idroot = m_pTree_DrawCalls->GetRootItem();

    m_pTree_DrawCalls->CollapseAndReset( idroot );
    m_DrawCallListDirty = true;
}

void PanelMemory::OnDrawCallTabSelected(wxNotebookEvent& event)
{
    RemoveAllDrawCalls();
}

void PanelMemory::OnDrawCallTreeSelectionChanged(wxTreeEvent& event)
{
    // TODO: fix this, it'll get called if any item in any "PanelMemory" tab is selected, not just the drawcall tab.
    m_DrawCallIndexToDraw = -1;
    wxTreeItemId id = event.GetItem();
    TreeItemDataVoidPtr* pData = (TreeItemDataVoidPtr*)m_pTree_DrawCalls->GetItemData( id );
    if( pData )
    {
        m_DrawCallIndexToDraw = (int)pData->m_ObjectPtr;
    }
}

void PanelMemory::UpdateRootNodeDrawCallCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_DrawCalls->GetRootItem();

    // update root node memory usage count.
    //sprintf_s( tempstr, 100, "Draws - size(%d)", g_pFileManager->CalculateTotalMemoryUsedByFiles() );
    //m_pTree_DrawCalls->SetItemText( idroot, tempstr );

    sprintf_s( tempstr, 100, "Draws(%d)",
        m_pTree_DrawCalls->GetChildrenCount( idroot, true ) - m_pTree_DrawCalls->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( 3, tempstr );
}
