//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MaterialDefinition.h"
#include "MaterialManager.h"
#include "../Core/GameCore.h"
#include "../Events/MyEvent.h"
#include "../Events/EventManager.h"
#include "../Events/EventTypeManager.h"
#include "../Helpers/FileManager.h"
#include "../Shaders/ShaderGroup.h"
#include "../Textures/TextureManager.h"

MaterialManager* g_pMaterialManager = nullptr;

MaterialManager::MaterialManager(GameCore* pGameCore)
{
    m_pGameCore = pGameCore;

#if MYFW_EDITOR
    m_pDefaultEditorMaterial = nullptr;
#endif

    m_pMaterialCreatedCallbackList.AllocateObjects( MAX_REGISTERED_CALLBACKS );
}

MaterialManager::~MaterialManager()
{
#if MYFW_EDITOR
    SAFE_RELEASE( m_pDefaultEditorMaterial );
#endif

    FreeAllMaterials();
    m_pMaterialCreatedCallbackList.FreeAllInList();
}

TextureManager* MaterialManager::GetTextureManager()
{
    return m_pGameCore->GetManagers()->GetTextureManager();
}

ShaderGroupManager* MaterialManager::GetShaderGroupManager()
{
    return m_pGameCore->GetManagers()->GetShaderGroupManager();
}

void MaterialManager::Tick()
{
    // TODO: Free file once the material is loaded... or not and check for updates on alt-tab.
    MaterialDefinition* pNextMaterial;
    for( MaterialDefinition* pMaterial = m_MaterialsStillLoading.GetHead(); pMaterial; pMaterial = pNextMaterial )
    {
        pNextMaterial = pMaterial->GetNext();

        MyAssert( pMaterial->m_pFile );

        if( pMaterial->m_pFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            if( strcmp( pMaterial->m_pFile->GetExtensionWithDot(), ".mymaterial" ) == 0 )
            {
                pMaterial->ImportFromFile();

                MyEvent* pEvent = g_pEventManager->CreateNewEvent( Event_MaterialFinishedLoading );
                g_pEventManager->SendEventNow( pEvent );
            }
            else
            {
                MyAssert( false );
            }
        }

        // File loading errors.
        if( pMaterial->m_pFile->GetFileLoadStatus() > FileLoadStatus_Success ) //== FileLoadStatus_Error_FileNotFound )
        {
            LOGError( LOGTag, "Material file failed to load: %s\n", pMaterial->m_pFile->GetFullPath() );
            
            // Move the material to the unsaved list.
            strcpy_s( pMaterial->m_Name, MaterialDefinition::MAX_MATERIAL_NAME_LEN, pMaterial->m_pFile->GetFilenameWithoutExtension() );

            m_Materials.MoveTail( pMaterial );
        }

        if( pMaterial->m_MaterialFileIsLoaded )
        {
            m_Materials.MoveTail( pMaterial );
        }
    }
}

void MaterialManager::FreeAllMaterials()
{
    while( MaterialDefinition* pMaterial = m_MaterialsStillLoading.GetHead() )
    {
        MyAssert( false );
        MyAssert( pMaterial->GetRefCount() == 1 );
        pMaterial->Release();
    }

    while( MaterialDefinition* pMaterial = m_Materials.GetHead() )
    {
        MyAssert( false );
        MyAssert( pMaterial->GetRefCount() == 1 );
        pMaterial->Release();
    }
}

#if MYFW_EDITOR
void MaterialManager::SaveAllMaterials(bool saveunchanged)
{
    for( MaterialDefinition* pMaterial = m_Materials.GetHead(); pMaterial; pMaterial = pMaterial->GetNext() )
    {
        if( pMaterial->m_UnsavedChanges || saveunchanged )
        {
            pMaterial->SaveMaterial( nullptr );
        }
    }
}

void MaterialManager::Editor_MoveMaterialToFrontOfLoadedList(MaterialDefinition* pMaterial)
{
    MyAssert( pMaterial->IsFullyLoaded() );
    MyAssert( pMaterial->GetPrev() );

    m_Materials.MoveHead( pMaterial );
}

MaterialDefinition* MaterialManager::GetDefaultEditorMaterial()
{
    if( m_pDefaultEditorMaterial == nullptr )
    {
        m_pDefaultEditorMaterial = CreateMaterial();
        ShaderGroup* pShader = MyNew ShaderGroup( GetShaderGroupManager(), GetTextureManager()->GetErrorTexture() );
        m_pDefaultEditorMaterial->SetShader( pShader );
        pShader->Release();
    }

    return m_pDefaultEditorMaterial;
}
#endif //MYFW_EDITOR

MaterialDefinition* MaterialManager::CreateMaterial(MyFileObject* pMaterialFile)
{
    MyAssert( pMaterialFile != nullptr );

    MaterialDefinition* pMaterial = MyNew MaterialDefinition( this );
    m_Materials.AddTail( pMaterial );

    pMaterial->SetFile( pMaterialFile );

    return pMaterial;
}

MaterialDefinition* MaterialManager::CreateMaterial(const char* name, const char* relativePath)
{
    MaterialDefinition* pMaterial = MyNew MaterialDefinition( this );
    m_Materials.AddTail( pMaterial );

    pMaterial->m_MaterialFileIsLoaded = true;
    if( name != nullptr )
    {
        pMaterial->m_UnsavedChanges = true;
        strcpy_s( pMaterial->m_Name, MaterialDefinition::MAX_MATERIAL_NAME_LEN, name );
    }

#if MYFW_EDITOR
    if( relativePath != nullptr )
    {
        pMaterial->SaveMaterial( relativePath );
        CallMaterialCreatedCallbacks( pMaterial );
    }
#endif

    return pMaterial;
}

MaterialDefinition* MaterialManager::LoadMaterial(const char* fullpath)
{
    MyAssert( fullpath );

    MaterialDefinition* pMaterial;

    // Check if this file was already loaded.
    pMaterial = FindMaterialByFilename( fullpath );
    if( pMaterial )
    {
        pMaterial->AddRef();
        return pMaterial;
    }

    pMaterial = MyNew MaterialDefinition( this );
    m_MaterialsStillLoading.AddTail( pMaterial );

    pMaterial->m_pFile = g_pFileManager->RequestFile( fullpath );

    MyAssert( pMaterial->m_pFile );

    return pMaterial;
}

void MaterialManager::ReloadMaterial(MaterialDefinition* pMaterial)
{
    MyAssert( pMaterial );
    MyAssert( pMaterial->m_pFile );
    MyAssert( pMaterial->m_pFile->GetFileLoadStatus() != FileLoadStatus_Success );

    m_MaterialsStillLoading.MoveTail( pMaterial );
    pMaterial->m_MaterialFileIsLoaded = false;
}

MaterialDefinition* MaterialManager::GetFirstMaterial()
{
    // Get first material that was loaded from disk.
    for( CPPListNode* pNode = m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->GetFile() != nullptr )
            return pMaterial;
    }

    // If there weren't any, just get the first material (if there is one).
    return (MaterialDefinition*)m_Materials.GetHead();
}

MaterialDefinition* MaterialManager::FindMaterial(ShaderGroup* m_pShaderGroup, TextureDefinition* pTextureColor)
{
    for( MaterialDefinition* pMaterial = m_MaterialsStillLoading.GetHead(); pMaterial; pMaterial = pMaterial->GetNext() )
    {
        if( pMaterial->m_pShaderGroup == m_pShaderGroup &&
            pMaterial->m_pTextureColor == pTextureColor )
        {
            return pMaterial;
        }
    }

    for( MaterialDefinition* pMaterial = m_Materials.GetHead(); pMaterial; pMaterial = pMaterial->GetNext() )
    {
        if( pMaterial->m_pShaderGroup == m_pShaderGroup &&
            pMaterial->m_pTextureColor == pTextureColor )
        {
            return pMaterial;
        }
    }

    return nullptr;
}

MaterialDefinition* MaterialManager::FindMaterialByFilename(const char* fullpath)
{
    for( MaterialDefinition* pMaterial = m_MaterialsStillLoading.GetHead(); pMaterial; pMaterial = pMaterial->GetNext() )
    {
        if( strcmp( pMaterial->m_pFile->GetFullPath(), fullpath ) == 0 )
            return pMaterial;
    }

    for( MaterialDefinition* pMaterial = m_Materials.GetHead(); pMaterial; pMaterial = pMaterial->GetNext() )
    {
        if( pMaterial->m_pFile && strcmp( pMaterial->m_pFile->GetFullPath(), fullpath ) == 0 )
            return pMaterial;
    }

    return nullptr;
}

void MaterialManager::RegisterMaterialCreatedCallback(void* pObj, MaterialCreatedCallbackFunc* pCallback)
{
    MyAssert( pObj != nullptr );
    MyAssert( pCallback != nullptr );
    MyAssert( m_pMaterialCreatedCallbackList.Count() < (unsigned int)MAX_REGISTERED_CALLBACKS );

    MaterialCreatedCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pMaterialCreatedCallbackList.Add( callbackstruct );
}

    //int id = evt.GetId();
    //if( id == 1000 )
    //{
    //    MaterialDefinition* pMaterial = g_pMaterialManager->CreateMaterial( "new" ); // the new material will only exist in the material manager.
    //    // TODO: this material will cause an assert on shutdown, unless released by some other code.

    //    // find the selected folder and put the object into that folder.
    //    wxString wxcategory = g_pPanelMemory->m_pTree_Materials->GetItemText( g_pMaterialManager->m_TreeIDRightClicked );
    //    const char* category = wxcategory;

    //    char tempstr[MAX_PATH];
    //    if( strcmp( category, "Materials" ) == 0 )
    //        sprintf_s( tempstr, MAX_PATH, "Data/Materials" );
    //    else
    //        sprintf_s( tempstr, MAX_PATH, "Data/Materials/%s", category );
    //    pMaterial->SaveMaterial( tempstr );

    //    g_pPanelMemory->RemoveMaterial( pMaterial );
    //    g_pPanelMemory->AddMaterial( pMaterial, category, pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
    //    g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, pMaterial, MaterialDefinition::StaticOnLabelEdit );

    //    CallMaterialCreatedCallbacks( pMaterial );
    //}

#if MYFW_EDITOR
void MaterialManager::CallMaterialCreatedCallbacks(MaterialDefinition* pMaterial)
{
    for( unsigned int i=0; i<g_pMaterialManager->m_pMaterialCreatedCallbackList.Count(); i++ )
    {
        g_pMaterialManager->m_pMaterialCreatedCallbackList[i].pFunc( g_pMaterialManager->m_pMaterialCreatedCallbackList[i].pObj, pMaterial );
    }
}
#endif
