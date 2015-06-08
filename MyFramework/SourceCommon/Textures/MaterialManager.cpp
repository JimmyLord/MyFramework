//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
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
    g_pPanelMemory->SetMaterialPanelCallbacks(this, MaterialManager::StaticOnLeftClick, MaterialManager::StaticOnRightClick, MaterialManager::StaticOnDrag);
#endif

    m_pMaterialCreatedCallbackList.AllocateObjects( MAX_REGISTERED_CALLBACKS );
}

MaterialManager::~MaterialManager()
{
    FreeAllMaterials();
    m_pMaterialCreatedCallbackList.FreeAllInList();
}

void MaterialManager::Tick()
{
    // TODO: free file once the material is loaded... or not and check for updates on alt-tab
    for( CPPListNode* pNode = m_MaterialsStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->m_pFile->m_FileLoadStatus == FileLoadStatus_Success )
        {
            pMaterial->ImportFromFile();

#if MYFW_USING_WX
            g_pPanelMemory->RemoveMaterial( pMaterial );
            g_pPanelMemory->AddMaterial( pMaterial, "Global", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
            g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );
#endif
        }

        if( pMaterial->m_FullyLoaded )
        {
            m_Materials.MoveTail( pMaterial );

            //if( pMaterial->m_pFile )
            //{
            //    g_pFileManager->FreeFile( pMaterial->m_pFile );
            //}
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
            pMaterial->SaveMaterial();
        }
    }
}
#endif

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

    for( unsigned int i=0; i<m_pMaterialCreatedCallbackList.Count(); i++ )
        m_pMaterialCreatedCallbackList[i].pFunc( m_pMaterialCreatedCallbackList[i].pObj, pMaterial );

#if MYFW_USING_WX
    if( name )
        g_pPanelMemory->AddMaterial( pMaterial, "Global", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
    else
        g_pPanelMemory->AddMaterial( pMaterial, "Temp materials", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
    g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );
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

#if MYFW_USING_WX
    g_pPanelMemory->AddMaterial( pMaterial, "Global", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
    g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );
#endif

    return pMaterial;
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

        if( strcmp( pMaterial->m_pFile->m_FullPath, fullpath ) == 0 )
            return pMaterial;
    }

    for( CPPListNode* pNode = m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->m_pFile && strcmp( pMaterial->m_pFile->m_FullPath, fullpath ) == 0 )
            return pMaterial;
    }

    return 0;
}

void MaterialManager::RegisterMaterialCreatedCallback(void* pObj, MaterialCreatedCallbackFunc pCallback)
{
    MyAssert( pCallback != 0 );
    MyAssert( m_pMaterialCreatedCallbackList.Count() < MAX_REGISTERED_CALLBACKS );

    MaterialCreatedCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pMaterialCreatedCallbackList.Add( callbackstruct );
}

#if MYFW_USING_WX
void MaterialManager::OnLeftClick(unsigned int count)
{
}

void MaterialManager::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

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
        g_pMaterialManager->CreateMaterial( "new" ); // the new material will only exist in the material manager.
        // TODO: this material will cause an assert on shutdown, unless released by some other code.
    }
}

void MaterialManager::OnDrag()
{
    //g_DragAndDropStruct.m_Type = DragAndDropType_MaterialDefinitionPointer;
    //g_DragAndDropStruct.m_Value = this;
}
#endif
