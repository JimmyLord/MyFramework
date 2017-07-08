//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "MaterialDefinition.h"
#include "MaterialManager.h"
#include "../Helpers/FileManager.h"

MaterialManager* g_pMaterialManager = 0;

MaterialManager::MaterialManager()
{
#if MYFW_USING_WX
    wxTreeItemId idroot = g_pPanelMemory->m_pTree_Materials->GetRootItem();
    g_pPanelMemory->SetMaterialPanelCallbacks( idroot, this, MaterialManager::StaticOnLeftClick, MaterialManager::StaticOnRightClick, MaterialManager::StaticOnDrag );

    m_pDefaultEditorMaterial = 0;
#endif

    m_pMaterialCreatedCallbackList.AllocateObjects( MAX_REGISTERED_CALLBACKS );
}

MaterialManager::~MaterialManager()
{
    SAFE_RELEASE( m_pDefaultEditorMaterial );

    FreeAllMaterials();
    m_pMaterialCreatedCallbackList.FreeAllInList();
}

void MaterialManager::Tick()
{
    // TODO: free file once the material is loaded... or not and check for updates on alt-tab
    for( CPPListNode* pNode = m_MaterialsStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        MyAssert( pMaterial->m_pFile );

        if( pMaterial->m_pFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            if( strcmp( pMaterial->m_pFile->GetExtensionWithDot(), ".mymaterial" ) == 0 )
            {
                pMaterial->ImportFromFile();

                MyEvent* pEvent = g_pEventManager->CreateNewEvent( Event_MaterialFinishedLoading );
                g_pEventManager->SendEventNow( pEvent );

#if MYFW_USING_WX
                const char* foldername = "Unknown";
                if( pMaterial->m_pFile )
                    foldername = pMaterial->m_pFile->GetNameOfDeepestFolderPath();

                g_pPanelMemory->RemoveMaterial( pMaterial );
                g_pPanelMemory->AddMaterial( pMaterial, foldername, pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
                g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );

                // Add right-click options to each material "folder".
                wxTreeItemId treeid = g_pPanelMemory->FindMaterialCategory( foldername );
                g_pPanelMemory->SetMaterialPanelCallbacks( treeid, this, MaterialManager::StaticOnLeftClick, MaterialManager::StaticOnRightClick, MaterialManager::StaticOnDrag );
#endif
            }
            else
            {
                MyAssert( false );
            }
        }

        // file loading errors
        if( pMaterial->m_pFile->GetFileLoadStatus() > FileLoadStatus_Success ) //== FileLoadStatus_Error_FileNotFound )
        {
            LOGError( LOGTag, "Material file failed to load: %s\n", pMaterial->m_pFile->GetFullPath() );
            
            // move the material to the unsaved list
            strcpy_s( pMaterial->m_Name, MaterialDefinition::MAX_MATERIAL_NAME_LEN, pMaterial->m_pFile->GetFilenameWithoutExtension() );

#if MYFW_USING_WX
            g_pPanelMemory->RemoveMaterial( pMaterial );
            g_pPanelMemory->AddMaterial( pMaterial, "Unsaved", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
            g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );
#endif // MYFW_USING_WX

            m_Materials.MoveTail( pMaterial );
        }

        if( pMaterial->m_FullyLoaded )
        {
            m_Materials.MoveTail( pMaterial );
        }
    }
}

void MaterialManager::FreeAllMaterials()
{
    while( CPPListNode* pNode = m_MaterialsStillLoading.GetHead() )
    {
        MyAssert( false );
        MyAssert( ((MaterialDefinition*)pNode)->GetRefCount() == 1 );
        ((MaterialDefinition*)pNode)->Release();
    }

    while( CPPListNode* pNode = m_Materials.GetHead() )
    {
        MyAssert( false );
        MyAssert( ((MaterialDefinition*)pNode)->GetRefCount() == 1 );
        ((MaterialDefinition*)pNode)->Release();
    }
}

#if MYFW_USING_WX
void MaterialManager::SaveAllMaterials(bool saveunchanged)
{
    for( CPPListNode* pNode = m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        //if( pMaterial->m_UnsavedChanges || saveunchanged )
        {
            pMaterial->SaveMaterial( 0 );
        }
    }
}

MaterialDefinition* MaterialManager::GetDefaultEditorMaterial()
{
    if( m_pDefaultEditorMaterial == 0 )
    {
        m_pDefaultEditorMaterial = CreateMaterial( "Placeholder Material" );
        ShaderGroup* pShader = MyNew ShaderGroup();
        m_pDefaultEditorMaterial->SetShader( pShader );
        pShader->Release();
    }

    return m_pDefaultEditorMaterial;
}
#endif

MaterialDefinition* MaterialManager::CreateMaterial(MyFileObject* pFile)
{
    MyAssert( pFile != 0 );

    MaterialDefinition* pMaterial = MyNew MaterialDefinition();
    m_Materials.AddTail( pMaterial );

    pMaterial->m_FullyLoaded = true;

    pMaterial->m_UnsavedChanges = true;
    strcpy_s( pMaterial->m_Name, MaterialDefinition::MAX_MATERIAL_NAME_LEN, pFile->GetFilenameWithoutExtension() );
    pMaterial->m_pFile = pFile;
    pMaterial->m_pFile->AddRef();

#if MYFW_USING_WX
    g_pPanelMemory->AddMaterial( pMaterial, "Unsaved", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
    g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );
#endif

    return pMaterial;
}

MaterialDefinition* MaterialManager::CreateMaterial(const char* name)
{
    MaterialDefinition* pMaterial = MyNew MaterialDefinition();
    m_Materials.AddTail( pMaterial );

    pMaterial->m_FullyLoaded = true;
    if( name != 0 )
    {
        pMaterial->m_UnsavedChanges = true;
        strcpy_s( pMaterial->m_Name, MaterialDefinition::MAX_MATERIAL_NAME_LEN, name );
    }

#if MYFW_USING_WX
    if( name != 0 )
    {
        g_pPanelMemory->AddMaterial( pMaterial, "Unsaved", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
        g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );
    }
#endif

    return pMaterial;
}

MaterialDefinition* MaterialManager::LoadMaterial(const char* fullpath)
{
    MyAssert( fullpath );

    MaterialDefinition* pMaterial;

    // check if this file was already loaded.
    pMaterial = FindMaterialByFilename( fullpath );
    if( pMaterial )
    {
        pMaterial->AddRef();
        return pMaterial;
    }

    pMaterial = MyNew MaterialDefinition();
    m_MaterialsStillLoading.AddTail( pMaterial );

    pMaterial->m_pFile = g_pFileManager->RequestFile( fullpath );

    MyAssert( pMaterial->m_pFile );

#if MYFW_USING_WX
    g_pPanelMemory->AddMaterial( pMaterial, "Loading", pMaterial->m_pFile->GetFilenameWithoutExtension(), MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
    g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );
#endif

    return pMaterial;
}

void MaterialManager::ReloadMaterial(MaterialDefinition* pMaterial)
{
    MyAssert( pMaterial );
    MyAssert( pMaterial->m_pFile );
    MyAssert( pMaterial->m_pFile->GetFileLoadStatus() != FileLoadStatus_Success );

    m_MaterialsStillLoading.MoveTail( pMaterial );
    pMaterial->m_FullyLoaded = false;
}

MaterialDefinition* MaterialManager::GetFirstMaterial()
{
    // Get first material that was loaded from disk
    for( CPPListNode* pNode = m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->GetFile() != 0 )
            return pMaterial;
    }

    // If there weren't any, just get the first material (if there is one).
    return (MaterialDefinition*)m_Materials.GetHead();
}

MaterialDefinition* MaterialManager::FindMaterial(ShaderGroup* m_pShaderGroup, TextureDefinition* pTextureColor)
{
    for( CPPListNode* pNode = m_MaterialsStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->m_pShaderGroup == m_pShaderGroup &&
            pMaterial->m_pTextureColor == pTextureColor )
        {
            return pMaterial;
        }
    }

    for( CPPListNode* pNode = m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->m_pShaderGroup == m_pShaderGroup &&
            pMaterial->m_pTextureColor == pTextureColor )
        {
            return pMaterial;
        }
    }

    return 0;
}

MaterialDefinition* MaterialManager::FindMaterialByFilename(const char* fullpath)
{
    for( CPPListNode* pNode = m_MaterialsStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( strcmp( pMaterial->m_pFile->GetFullPath(), fullpath ) == 0 )
            return pMaterial;
    }

    for( CPPListNode* pNode = m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->m_pFile && strcmp( pMaterial->m_pFile->GetFullPath(), fullpath ) == 0 )
            return pMaterial;
    }

    return 0;
}

void MaterialManager::RegisterMaterialCreatedCallback(void* pObj, MaterialCreatedCallbackFunc pCallback)
{
    MyAssert( pCallback != 0 );
    MyAssert( m_pMaterialCreatedCallbackList.Count() < (unsigned int)MAX_REGISTERED_CALLBACKS );

    MaterialCreatedCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pMaterialCreatedCallbackList.Add( callbackstruct );
}

#if MYFW_USING_WX
void MaterialManager::OnLeftClick(unsigned int count)
{
}

void MaterialManager::OnRightClick(wxTreeItemId treeid)
{
 	wxMenu menu;
    menu.SetClientData( this );

    m_TreeIDRightClicked = treeid;

    menu.Append( 1000, "Create new material" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&MaterialManager::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void MaterialManager::OnPopupClick(wxEvent &evt)
{
    int id = evt.GetId();
    if( id == 1000 )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->CreateMaterial( "new" ); // the new material will only exist in the material manager.
        // TODO: this material will cause an assert on shutdown, unless released by some other code.

#if MYFW_USING_WX
        // find the selected folder and put the object into that folder.
        wxString wxcategory = g_pPanelMemory->m_pTree_Materials->GetItemText( g_pMaterialManager->m_TreeIDRightClicked );
        const char* category = wxcategory;

        char tempstr[MAX_PATH];
        if( strcmp( category, "Materials" ) == 0 )
            sprintf_s( tempstr, MAX_PATH, "Data/Materials" );
        else
            sprintf_s( tempstr, MAX_PATH, "Data/Materials/%s", category );
        pMaterial->SaveMaterial( tempstr );

        g_pPanelMemory->RemoveMaterial( pMaterial );
        g_pPanelMemory->AddMaterial( pMaterial, category, pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
        g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );

        for( unsigned int i=0; i<g_pMaterialManager->m_pMaterialCreatedCallbackList.Count(); i++ )
            g_pMaterialManager->m_pMaterialCreatedCallbackList[i].pFunc( g_pMaterialManager->m_pMaterialCreatedCallbackList[i].pObj, pMaterial );
#endif
    }
}

void MaterialManager::OnDrag()
{
    //g_DragAndDropStruct.m_Type = DragAndDropType_MaterialDefinitionPointer;
    //g_DragAndDropStruct.m_Value = this;
}
#endif
