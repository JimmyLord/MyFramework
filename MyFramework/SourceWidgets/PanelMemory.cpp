//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"
#include "PanelMemory.h"

PanelMemory* g_pPanelMemory = 0;

PanelMemory::PanelMemory(wxFrame* parentframe)
: wxPanel( parentframe, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER, "Memory" )
{
    wxTreeItemId idroot;

    // Create a notebook with 7 pages(materials/textures/shaders/sound cues/files/buffers/draws).
    m_pNotebook = MyNew wxNotebook( this, wxID_ANY, wxPoint(0,0), wxDefaultSize );

    m_pTree_Materials = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_EDIT_LABELS );// | wxTR_MULTIPLE );
    idroot = m_pTree_Materials->AddRoot( "Materials" );
    m_pNotebook->AddPage( m_pTree_Materials, "Mat" );
    
    m_pTree_Textures = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize );
    idroot = m_pTree_Textures->AddRoot( "Textures" );
    m_pNotebook->AddPage( m_pTree_Textures, "Tex" );

    m_pTree_ShaderGroups = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize );
    idroot = m_pTree_ShaderGroups->AddRoot( "Shaders" );
    m_pNotebook->AddPage( m_pTree_ShaderGroups, "Shaders" );

    m_pTree_SoundCues = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_EDIT_LABELS | wxTR_MULTIPLE );
    idroot = m_pTree_SoundCues->AddRoot( "Sound Cues" );
    m_pNotebook->AddPage( m_pTree_SoundCues, "Cues" );

    m_pTree_Files = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize );
    idroot = m_pTree_Files->AddRoot( "Files" );
    m_pNotebook->AddPage( m_pTree_Files, "Files" );

    m_pTree_Buffers = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize );
    idroot = m_pTree_Buffers->AddRoot( "Buffers" );
    m_pNotebook->AddPage( m_pTree_Buffers, "Buffers" );

    m_pTree_DrawCalls = MyNew wxTreeCtrl( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize );
    idroot = m_pTree_DrawCalls->AddRoot( "Draws" );
    m_pNotebook->AddPage( m_pTree_DrawCalls, "Draws" );

    // Setup sizers to resize the notebook in both directions.
    wxBoxSizer* verticalsizer = MyNew wxBoxSizer( wxVERTICAL );
    wxBoxSizer* horizontalsizer = MyNew wxBoxSizer( wxHORIZONTAL );
    horizontalsizer->Add( m_pNotebook, 1, wxGROW|wxALL, 2 );
    verticalsizer->Add( horizontalsizer, 1, wxGROW|wxALL, 2 );
    SetSizerAndFit( verticalsizer );

    Update();

    Connect( wxEVT_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( PanelMemory::OnTabSelected ) );
    Connect( wxEVT_TREE_SEL_CHANGED, wxTreeEventHandler( PanelMemory::OnTreeSelectionChanged ) );
    Connect( wxEVT_TREE_BEGIN_LABEL_EDIT, wxTreeEventHandler( PanelMemory::OnTreeBeginLabelEdit ) );
    Connect( wxEVT_TREE_END_LABEL_EDIT, wxTreeEventHandler( PanelMemory::OnTreeEndLabelEdit ) );
    Connect( wxEVT_TREE_ITEM_MENU, wxTreeEventHandler( PanelMemory::OnTreeContextMenuRequested ) );
    Connect( wxEVT_TREE_BEGIN_DRAG, wxTreeEventHandler( PanelMemory::OnDragBegin ) );
    Connect( wxEVT_TREE_KEY_DOWN, wxTreeEventHandler( PanelMemory::OnKeyDown ) );

    m_DrawCallListDirty = false;
    m_DrawCallIndexToDraw = -1;
}

PanelMemory::~PanelMemory()
{
    // Remove all pages before deleting them.
    while( m_pNotebook->GetPageCount() )
        m_pNotebook->RemovePage( 0 );

    // Delete all pages/trees.
    SAFE_DELETE( m_pTree_Materials )
    SAFE_DELETE( m_pTree_Textures );
    SAFE_DELETE( m_pTree_ShaderGroups );
    SAFE_DELETE( m_pTree_SoundCues );
    SAFE_DELETE( m_pTree_Files );
    SAFE_DELETE( m_pTree_Buffers );
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
            void* objptr = ((TreeItemDataGenericObjectInfo*)pData)->m_pObject;
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
//        //    void* objptr = ((TreeItemDataGenericObjectInfo*)pData)->m_ptr;
//        //    if( objptr == pBufferDef )
//        //        break;
//
//        //    id = m_pTree_Buffers->GetNextChild( idroot, cookie );
//        //}
//
//        //if( id.IsOk() == false )
//        {
//            sprintf_s( tempstr, 100, "Buffer %d - size(%d) - num(%d)", count, pBufferDef->m_DataSize, pBufferDef->m_NumBuffersToUse );
//            TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
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
    MyAssert( pBufferDef != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Buffers->GetRootItem();
    int count = (int)m_pTree_Buffers->GetChildrenCount( idroot, false );

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

    int categorycount = (int)m_pTree_Buffers->GetChildrenCount( idcategory );

    // insert the buffer into it's category
    {
#if MYFW_WINDOWS
        if( category == 0 || category[0] == 0 || desc == 0 || desc[0] == 0 )
            __debugbreak();
#endif

        sprintf_s( tempstr, 100, "%s %d - size(%d) - num(%d)", desc, categorycount, pBufferDef->m_DataSize, pBufferDef->m_NumBuffersToUse );
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pBufferDef;

        m_pTree_Buffers->AppendItem( idcategory, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
            m_pTree_Buffers->Expand( idroot );
    }

    UpdateRootNodeBufferCount();
}

void PanelMemory::UpdateBuffer(BufferDefinition* pBufferDef)
{
    wxTreeItemId idroot = m_pTree_Buffers->GetRootItem();

    //wxTreeItemId id =
    FindObject( m_pTree_Buffers, pBufferDef, idroot );

    //m_pTree_Buffers->
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

    sprintf_s( tempstr, 100, "Buffs(%d)",
        (int)m_pTree_Buffers->GetChildrenCount( idroot, true ) - (int)m_pTree_Buffers->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( PanelMemoryPage_Buffers, tempstr );
}

void PanelMemory::AddTexture(TextureDefinition* pTextureDef, const char* category, const char* desc, PanelObjectListObjectCallbackRightClick* pRightClickFunction, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pTextureDef != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Textures->GetRootItem();
    int count = (int)m_pTree_Textures->GetChildrenCount( idroot, false );

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

    int categorycount = (int)m_pTree_Textures->GetChildrenCount( idcategory );

    // insert the Texture into it's category
    {
        sprintf_s( tempstr, 100, "%s %d - size(%d) - fileinmemory(%d)", desc, categorycount, pTextureDef->GetMemoryUsed(), pTextureDef->GetFile()?1:0 );
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pTextureDef;
        pData->m_pRightClickFunction = pRightClickFunction;
        pData->m_pDragFunction = pDragFunction;

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
    //    0 );//EngineCore->GetManagers()->GetTextureManager()->CalculateTotalMemoryUsedByTextures() );
    //m_pTree_Textures->SetItemText( idroot, tempstr );
    sprintf_s( tempstr, 100, "Tex(%d)",
        (int)m_pTree_Textures->GetChildrenCount( idroot, true ) - (int)m_pTree_Textures->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( PanelMemoryPage_Textures, tempstr );
}

wxTreeItemId PanelMemory::FindMaterialCategory(const char* category)
{
    wxTreeItemId idroot = m_pTree_Materials->GetRootItem();

    // see if the category exists
    wxTreeItemId idcategory;

    wxTreeItemIdValue cookie;
    idcategory = m_pTree_Materials->GetFirstChild( idroot, cookie );
    while( idcategory.IsOk() )
    {
        wxString catstr = m_pTree_Materials->GetItemText( idcategory );
        if( catstr == category )
            break;

        idcategory = m_pTree_Materials->GetNextChild( idroot, cookie );
    }

    return idcategory;
}

void PanelMemory::AddMaterial(MaterialDefinition* pMaterial, const char* category, const char* desc, PanelObjectListObjectCallbackLeftClick* pLeftClickFunction, PanelObjectListObjectCallbackRightClick* pRightClickFunction, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pMaterial != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Materials->GetRootItem();
    int count = (int)m_pTree_Materials->GetChildrenCount( idroot, false );

    // see if the category exists
    wxTreeItemId idcategory = FindMaterialCategory( category );
    
    // insert the category if necessary
    if( idcategory.IsOk() == false )
    {
        idcategory = m_pTree_Materials->AppendItem( idroot, category, -1, -1, 0 );
    }

    //int categorycount = (int)m_pTree_Materials->GetChildrenCount( idcategory );

    // insert the Material into it's category
    {
        sprintf_s( tempstr, 100, "%s", desc );
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pMaterial;
        pData->m_pLeftClickFunction = pLeftClickFunction;
        pData->m_pRightClickFunction = pRightClickFunction;
        pData->m_pDragFunction = pDragFunction;

        m_pTree_Materials->AppendItem( idcategory, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
            m_pTree_Materials->Expand( idroot );
    }

    UpdateRootNodeMaterialCount();
}

void PanelMemory::RemoveMaterial(MaterialDefinition* pMaterial)
{
    wxTreeItemId idroot = m_pTree_Materials->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_Materials, pMaterial, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        wxTreeItemId parentid = m_pTree_Materials->GetItemParent( id );

        m_pTree_Materials->Delete( id );

        if( m_pTree_Materials->GetChildrenCount( parentid ) == 0 )
            m_pTree_Materials->Delete( parentid );

        UpdateRootNodeMaterialCount();
    }
}

void PanelMemory::SetMaterialPanelCallbacks(wxTreeItemId treeid, void* pObject, PanelObjectListObjectCallbackLeftClick* pLeftClickFunction, PanelObjectListObjectCallbackRightClick* pRightClickFunction, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pObject != 0 );

    if( treeid.IsOk() )
    {
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Materials->GetItemData( treeid );
        if( pData == 0 )
            pData = MyNew TreeItemDataGenericObjectInfo();

        pData->m_pObject = pObject;
        pData->m_pLeftClickFunction = pLeftClickFunction;
        pData->m_pRightClickFunction = pRightClickFunction;
        pData->m_pDragFunction = pDragFunction;

        m_pTree_Materials->SetItemData( treeid, pData );
    }
}

const char* PanelMemory::GetSelectedMaterialTreeItemText()
{
    wxTreeItemId id = m_pTree_Materials->GetSelection();

    if( id.IsOk() )
    {
        wxString catstr = m_pTree_Materials->GetItemText( id );
        return catstr;
    }

    return 0;
}

MaterialDefinition* PanelMemory::GetSelectedMaterial()
{
    wxTreeItemId id = m_pTree_Materials->GetSelection();
    wxTreeItemId idroot = m_pTree_Materials->GetRootItem();

    if( id.IsOk() && id != idroot )
    {
        wxTreeItemData* pData = m_pTree_Materials->GetItemData( id );
        if( pData )
        {
            MaterialDefinition* objptr = (MaterialDefinition*)((TreeItemDataGenericObjectInfo*)pData)->m_pObject;
            if( (void*)objptr != m_pMaterialManager )
                return objptr;
        }
    }

    return 0;
}

void PanelMemory::UpdateRootNodeMaterialCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Materials->GetRootItem();

    // update root node memory usage count.
    //sprintf_s( tempstr, 100, "Materials(%d) - size(%d)",
    //    m_pTree_Materials->GetChildrenCount( idroot, true ) - m_pTree_Materials->GetChildrenCount( idroot, false ),
    //    0 );//m_pMaterialManager->CalculateTotalMemoryUsedByMaterials() );
    //m_pTree_Materials->SetItemText( idroot, tempstr );
    sprintf_s( tempstr, 100, "Mat(%d)",
        (int)m_pTree_Materials->GetChildrenCount( idroot, true ) - (int)m_pTree_Materials->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( PanelMemoryPage_Materials, tempstr );
}

wxTreeItemId PanelMemory::FindFile(MyFileObject* pFile)
{
    wxTreeItemId idroot = m_pTree_Files->GetRootItem();
    wxTreeItemId id = FindObject( m_pTree_Files, pFile, idroot );

    return id;
}

void PanelMemory::AddFile(MyFileObject* pFile, const char* category, const char* desc, PanelObjectListObjectCallbackLeftClick* pLeftClickFunction, PanelObjectListObjectCallbackRightClick* pRightClickFunction, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pFile != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_Files->GetRootItem();
    int count = (int)m_pTree_Files->GetChildrenCount( idroot, false );

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

    int categorycount = (int)m_pTree_Files->GetChildrenCount( idcategory );

    // insert the File into it's category
    {
        sprintf_s( tempstr, 100, "%s %d - size(%d)", desc, categorycount, pFile->GetFileLength() );
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pFile;
        pData->m_pLeftClickFunction = pLeftClickFunction;
        pData->m_pRightClickFunction = pRightClickFunction;
        pData->m_pDragFunction = pDragFunction;

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

void PanelMemory::SetFilePanelCallbacks(wxTreeItemId treeid, void* pObject, PanelObjectListObjectCallbackLeftClick* pLeftClickFunction, PanelObjectListObjectCallbackRightClick* pRightClickFunction, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pObject != 0 );

    if( treeid.IsOk() )
    {
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_Files->GetItemData( treeid );
        if( pData == 0 )
            pData = MyNew TreeItemDataGenericObjectInfo();

        pData->m_pObject = pObject;
        pData->m_pLeftClickFunction = pLeftClickFunction;
        pData->m_pRightClickFunction = pRightClickFunction;
        pData->m_pDragFunction = pDragFunction;

        m_pTree_Files->SetItemData( treeid, pData );
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
        (int)m_pTree_Files->GetChildrenCount( idroot, true ) - (int)m_pTree_Files->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( PanelMemoryPage_Files, tempstr );
}

void PanelMemory::AddDrawCall(int index, const char* category, const char* desc)
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_DrawCalls->GetRootItem();
    int count = (int)m_pTree_DrawCalls->GetChildrenCount( idroot, false );

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

    int categorycount = (int)m_pTree_DrawCalls->GetChildrenCount( idcategory );

    // insert the DrawCall into it's category
    {
        sprintf_s( tempstr, 100, "%s %d - size(%d)", desc, categorycount, 1 );
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = (void*)(long)index; //TODO: fix?

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

    wxTreeItemId id = FindObject( m_pTree_DrawCalls, (void*)(long)index, idroot );

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

void PanelMemory::OnTabSelected(wxNotebookEvent& event)
{
    RemoveAllDrawCalls();
}

void PanelMemory::OnTreeSelectionChanged(wxTreeEvent& event)
{
    // get the pointer to the tree affected.
    wxTreeCtrl* pTree = (wxTreeCtrl*)event.GetEventObject();

    //// if any material in the material tab is selected, do nothing...
    //if( pTree == m_pTree_Materials )
    //{
    //}

    // if any item in the draw call tab is selected, set m_DrawCallIndexToDraw.
    if( pTree == m_pTree_DrawCalls )
    {
        m_DrawCallIndexToDraw = -1;
        wxTreeItemId id = event.GetItem();
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_DrawCalls->GetItemData( id );
        if( pData )
        {
            // pData->m_pObject is a pointer that stores an int,
            //    it should never be larger than a few thousand let alone 2 billion, so typecast is ugly but will work.
            m_DrawCallIndexToDraw = (int)(uintptr_t)pData->m_pObject;
        }
    }

    // if there's a callback for left clicks, call it. // used by MyFileObjects ATM.
    {
        wxTreeItemId id = event.GetItem();
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_DrawCalls->GetItemData( id );
        if( pData && pData->m_pLeftClickFunction )
        {
            pData->m_pLeftClickFunction( pData->m_pObject, id, 0 );
        }
    }
}

void PanelMemory::OnTreeContextMenuRequested(wxTreeEvent& event)
{
    // get the pointer to the tree affected.
    wxTreeCtrl* pTree = (wxTreeCtrl*)event.GetEventObject();

    // pass right click events through to the item.
    wxTreeItemId id = event.GetItem();
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)pTree->GetItemData( id );
    if( pData && pData->m_pRightClickFunction )
    {
        pData->m_pRightClickFunction( pData->m_pObject, id );
    }
}

void PanelMemory::OnDragBegin(wxTreeEvent& event)
{
    // let the object know its being dragged, so it can store it's data.
    // This only works within this app, not between apps.

    // get the pointer to the tree affected.
    wxTreeCtrl* pTree = (wxTreeCtrl*)event.GetEventObject();

    // Clear out the hacked up Drag and Drag structure, so the m_pDragFunction callback can fill it up.
    g_DragAndDropStruct.Clear();

    wxTreeItemId id = event.GetItem();
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)pTree->GetItemData( id );
    if( pData && pData->m_pDragFunction )
    {
        pData->m_pDragFunction( pData->m_pObject );
    }
    else
    {
        return; // Cancel drag and drop.
    }

    // Dummy data to kick off the drag/drop op.  Real data is handled by objects in list.
#if MYFW_WINDOWS
    wxCustomDataObject dataobject;
    dataobject.SetFormat( *g_pMyDataFormat );
    wxDropSource dragsource( dataobject );
#else //elif MYFW_OSX
    wxFileDataObject dataobject; // Using wxCustomDataObject doesn't work on OSX, TODO: look deeper
    dataobject.SetFormat( *g_pMyDataFormat );
    wxDropSource dragsource( this );
    dragsource.SetData( dataobject );
#endif
    wxDragResult result = dragsource.DoDragDrop( wxDrag_CopyOnly );
}

void PanelMemory::UpdateRootNodeDrawCallCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_DrawCalls->GetRootItem();

    // update root node memory usage count.
    //sprintf_s( tempstr, 100, "Draws - size(%d)", g_pFileManager->CalculateTotalMemoryUsedByFiles() );
    //m_pTree_DrawCalls->SetItemText( idroot, tempstr );

    sprintf_s( tempstr, 100, "Draws(%d)",
        (int)m_pTree_DrawCalls->GetChildrenCount( idroot, true ) - (int)m_pTree_DrawCalls->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( PanelMemoryPage_DrawCalls, tempstr );
}

void PanelMemory::AddShaderGroup(ShaderGroup* pShaderGroup, const char* category, const char* desc, PanelObjectListObjectCallbackRightClick* pRightClickFunction, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pShaderGroup != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_ShaderGroups->GetRootItem();
    int count = (int)m_pTree_ShaderGroups->GetChildrenCount( idroot, false );

    // see if the category exists
    wxTreeItemId idcategory;
    {
        wxTreeItemIdValue cookie;
        idcategory = m_pTree_ShaderGroups->GetFirstChild( idroot, cookie );
        while( idcategory.IsOk() )
        {
            wxString catstr = m_pTree_ShaderGroups->GetItemText( idcategory );
            if( catstr == category )
                break;

            idcategory = m_pTree_ShaderGroups->GetNextChild( idroot, cookie );
        }
    }
    
    // insert the category if necessary
    if( idcategory.IsOk() == false )
    {
        idcategory = m_pTree_ShaderGroups->AppendItem( idroot, category, -1, -1, 0 );
    }

    int categorycount = (int)m_pTree_ShaderGroups->GetChildrenCount( idcategory );

    // insert the ShaderGroup into it's category
    {
        sprintf_s( tempstr, 100, "%s %d", desc, categorycount );
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pShaderGroup;
        pData->m_pDragFunction = pDragFunction;
        pData->m_pRightClickFunction = pRightClickFunction;

        m_pTree_ShaderGroups->AppendItem( idcategory, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
            m_pTree_ShaderGroups->Expand( idroot );
    }

    UpdateRootNodeShaderGroupCount();
}

void PanelMemory::RemoveShaderGroup(ShaderGroup* pShaderGroup)
{
    wxTreeItemId idroot = m_pTree_ShaderGroups->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_ShaderGroups, pShaderGroup, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        wxTreeItemId parentid = m_pTree_ShaderGroups->GetItemParent( id );

        m_pTree_ShaderGroups->Delete( id );

        if( m_pTree_ShaderGroups->GetChildrenCount( parentid ) == 0 )
            m_pTree_ShaderGroups->Delete( parentid );

        UpdateRootNodeShaderGroupCount();
    }
}

void PanelMemory::UpdateRootNodeShaderGroupCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_ShaderGroups->GetRootItem();

    // update root node memory usage count.
    sprintf_s( tempstr, 100, "Shaders" );
    m_pTree_ShaderGroups->SetItemText( idroot, tempstr );

    sprintf_s( tempstr, 100, "Shaders(%d)",
        (int)m_pTree_ShaderGroups->GetChildrenCount( idroot, true ) - (int)m_pTree_ShaderGroups->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( PanelMemoryPage_ShaderGroups, tempstr );
}

wxTreeItemId PanelMemory::FindSoundCue(SoundCue* pSoundCue)
{
    wxTreeItemId idroot = m_pTree_SoundCues->GetRootItem();
    wxTreeItemId id = FindObject( m_pTree_SoundCues, pSoundCue, idroot );

    return id;
}

wxTreeItemId PanelMemory::AddSoundCue(SoundCue* pSoundCue, const char* category, const char* desc, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pSoundCue != 0 );

    char tempstr[100];

    wxTreeItemId idroot = m_pTree_SoundCues->GetRootItem();
    int count = (int)m_pTree_SoundCues->GetChildrenCount( idroot, false );

    wxTreeItemId newid;

    // insert the SoundCue
    {
        sprintf_s( tempstr, 100, "%s", desc );
        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pSoundCue;
        pData->m_pDragFunction = pDragFunction;

        newid = m_pTree_SoundCues->AppendItem( idroot, tempstr, -1, -1, pData );

        // if inserting the first item, then expand the tree.
        if( count == 0 )
            m_pTree_SoundCues->Expand( idroot );
    }

    UpdateRootNodeSoundCueCount();

    return newid;
}

void PanelMemory::RemoveSoundCue(SoundCue* pSoundCue)
{
    wxTreeItemId idroot = m_pTree_SoundCues->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_SoundCues, pSoundCue, idroot );

    if( id.IsOk() )
    {
        // delete item and up to one parent... should be fully recursive up the chain(ignoring root) but it's not for now.
        //wxTreeItemId parentid = m_pTree_SoundCues->GetItemParent( id );

        m_pTree_SoundCues->Delete( id );

        //if( m_pTree_SoundCues->GetChildrenCount( parentid ) == 0 )
        //    m_pTree_SoundCues->Delete( parentid );

        UpdateRootNodeSoundCueCount();
    }
}

void PanelMemory::UpdateRootNodeSoundCueCount()
{
    char tempstr[100];

    wxTreeItemId idroot = m_pTree_SoundCues->GetRootItem();

    // update root node memory usage count.
    sprintf_s( tempstr, 100, "Cues" );
    m_pTree_SoundCues->SetItemText( idroot, tempstr );

    sprintf_s( tempstr, 100, "Cues(%d)",
        (int)m_pTree_SoundCues->GetChildrenCount( idroot, true ) - (int)m_pTree_SoundCues->GetChildrenCount( idroot, false ) );
    m_pNotebook->SetPageText( PanelMemoryPage_SoundCues, tempstr );
}

wxTreeItemId PanelMemory::AddSoundObject(SoundObject* pSound, SoundCue* pSoundCue, const char* desc, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pSound != 0 );
    MyAssert( pSoundCue != 0 );

    // Find the sound cue
    wxTreeItemId idroot = m_pTree_SoundCues->GetRootItem();
    wxTreeItemId idcue = FindObject( m_pTree_SoundCues, pSoundCue, idroot );

    wxTreeItemId newid;

    // the cue should exist, we can't add a sound without one.
    if( idcue.IsOk() == false )
    {
        MyAssert( false );
        return newid;
    }

    // insert the SoundCue into it's category
    {
        char tempstr[MAX_PATH];
        sprintf_s( tempstr, MAX_PATH, "%s", desc );

        TreeItemDataGenericObjectInfo* pData = MyNew TreeItemDataGenericObjectInfo();
        pData->m_pObject = pSoundCue;
        pData->m_pDragFunction = pDragFunction;

        newid = m_pTree_SoundCues->AppendItem( idcue, tempstr, -1, -1, pData );
    }

    UpdateRootNodeSoundCueCount();

    return newid;
}

void PanelMemory::RemoveSoundObject(SoundObject* pSound)
{
    MyAssert( pSound != 0 );

    wxTreeItemId idroot = m_pTree_SoundCues->GetRootItem();

    wxTreeItemId id = FindObject( m_pTree_SoundCues, pSound, idroot );

    if( id.IsOk() )
    {
        m_pTree_SoundCues->Delete( id );

        UpdateRootNodeSoundCueCount();
    }
}

void PanelMemory::SetSoundPanelCallbacks(wxTreeItemId treeid, void* pObject, PanelObjectListObjectCallbackLeftClick* pLeftClickFunction, PanelObjectListObjectCallbackRightClick* pRightClickFunction, PanelObjectListObjectCallback* pDragFunction)
{
    MyAssert( pObject != 0 );

    if( treeid.IsOk() )
    {
        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)m_pTree_SoundCues->GetItemData( treeid );
        if( pData == 0 )
            pData = MyNew TreeItemDataGenericObjectInfo();

        pData->m_pObject = pObject;
        pData->m_pLeftClickFunction = pLeftClickFunction;
        pData->m_pRightClickFunction = pRightClickFunction;
        pData->m_pDragFunction = pDragFunction;

        m_pTree_Materials->SetItemData( treeid, pData );
    }
}

void PanelMemory::SetLabelEditFunction(wxTreeCtrl* pTree, void* pObject, PanelObjectListObjectCallbackLabelEdit* pLabelEditFunction)
{
    wxTreeItemId idroot = pTree->GetRootItem();
    wxTreeItemId id = FindObject( pTree, pObject, idroot );
    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)pTree->GetItemData( id );
    pData->m_pLabelEditFunction = pLabelEditFunction;
}

void PanelMemory::OnTreeBeginLabelEdit(wxTreeEvent& event)
{
    wxTreeCtrl* pTree = (wxTreeCtrl*)event.GetEventObject();
    wxTreeItemId id = event.GetItem();

    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)pTree->GetItemData( id );

    // don't allow edits if there's no callback.
    if( pData == 0 || pData->m_pLabelEditFunction == 0 )
    {
        // cancel the edit
        event.Veto();
    }
}

void PanelMemory::OnTreeEndLabelEdit(wxTreeEvent& event)
{
    wxTreeCtrl* pTree = (wxTreeCtrl*)event.GetEventObject();
    wxTreeItemId id = event.GetItem();

    TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)pTree->GetItemData( id );

    if( pData && pData->m_pLabelEditFunction )
    {
        wxString newlabel = event.GetLabel();

        if( event.IsEditCancelled() == false )
        {
            // Manually set the item to the new name, so the callback func can query for the name.
            //    The actual label edit process seems to happen later.
            RenameObject( pTree, pData->m_pObject, newlabel );
            
            // Since the new label was forced into the edit box by the call above,
            //    we'll cancel the label edit here so the callback can rename it again if needed.
            event.Veto();

            // Call the callback and let game code handle the new name.
            pData->m_pLabelEditFunction( pData->m_pObject, id, newlabel );
        }
    }
}

void PanelMemory::OnKeyDown(wxTreeEvent& event)
{
    if( event.GetKeyCode() == WXK_F2 )
    {
        wxTreeCtrl* pTree = (wxTreeCtrl*)event.GetEventObject();

        wxArrayTreeItemIds selecteditems;
        unsigned int numselected = (unsigned int)pTree->GetSelections( selecteditems );

        if( numselected == 1 )
        {
            wxTreeItemId id = selecteditems[0].GetID();
            pTree->EditLabel( id );
        }

        return;
    }

    if( event.GetKeyCode() == WXK_DELETE )
    {
        //return;
    }

    event.Skip();
}

wxString PanelMemory::GetObjectName(wxTreeCtrl* pTree, void* pObject)
{
    wxTreeItemId idroot = pTree->GetRootItem();

    wxTreeItemId id = FindObject( pTree, pObject, idroot );

    if( id.IsOk() )
    {
        return pTree->GetItemText( id );
    }

    return "";
}

void PanelMemory::RenameObject(wxTreeCtrl* pTree, void* pObject, const char* desc)
{
    wxTreeItemId idroot = pTree->GetRootItem();

    wxTreeItemId id = FindObject( pTree, pObject, idroot );

    if( id.IsOk() )
    {
        pTree->SetItemText( id, desc );
    }

    //UpdateRootNodeObjectCount();
}
