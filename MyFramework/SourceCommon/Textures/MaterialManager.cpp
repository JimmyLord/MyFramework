//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "MaterialManager.h"
#include "../Helpers/FileManager.h"

MaterialManager* g_pMaterialManager = 0;

MaterialDefinition::MaterialDefinition()
{
    m_FullyLoaded = false;

    m_UnsavedChanges = false;

    m_Name[0] = 0;
    m_pFile = 0;

    m_pShaderGroup = 0;
    m_pTextureColor = 0;
}

MaterialDefinition::~MaterialDefinition()
{
    SAFE_RELEASE( m_pTextureColor );

    if( m_pFile )
    {
        g_pFileManager->FreeFile( m_pFile );
    }
}

void MaterialDefinition::ImportFromFile()
{
    // TODO: replace asserts: if a shader or texture isn't found, load it.

    assert( m_pFile && m_pFile->m_FileLoadStatus == FileLoadStatus_Success );
    if( m_pFile == 0 || m_pFile->m_FileLoadStatus != FileLoadStatus_Success )
        return;

    cJSON* jsonobj = cJSON_Parse( m_pFile->m_pBuffer );

    cJSON* shaderstringobj = cJSON_GetObjectItem( jsonobj, "Shader" );
    if( shaderstringobj )
    {
        ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByName( shaderstringobj->valuestring );
        assert( pShaderGroup ); // fix
        if( pShaderGroup )
        {
            pShaderGroup->AddRef();
            SAFE_RELEASE( m_pShaderGroup );
            m_pShaderGroup = pShaderGroup;
        }
    }

    cJSON* texcolorstringobj = cJSON_GetObjectItem( jsonobj, "TexColor" );
    if( texcolorstringobj )
    {
        TextureDefinition* pTexture = g_pTextureManager->FindTexture( texcolorstringobj->valuestring );
        assert( pTexture ); // fix
        if( pTexture )
        {
            pTexture->AddRef();
            SAFE_RELEASE( m_pTextureColor );
            m_pTextureColor = pTexture;
        }
    }

    m_FullyLoaded = true;
}

#if MYFW_USING_WX
void MaterialDefinition::OnLeftClick()
{
}

void MaterialDefinition::OnRightClick()
{
}

void MaterialDefinition::OnDrag()
{
    //g_DragAndDropStruct.m_Type = DragAndDropType_MaterialDefinitionPointer;
    //g_DragAndDropStruct.m_Value = this;
}

void MaterialDefinition::SaveMaterial()
{
    char filename[MAX_PATH];
    char workingdir[MAX_PATH];
#if MYFW_WINDOWS
    _getcwd( workingdir, MAX_PATH * sizeof(char) );
#else
    getcwd( workingdir, MAX_PATH * sizeof(char) );
#endif
    sprintf_s( filename, MAX_PATH, "%s%s%s", workingdir, m_Name, ".mymaterial" );

    cJSON* root = cJSON_CreateObject();

    cJSON* material = cJSON_CreateObject();
    cJSON_AddItemToObject( root, "Material", material );

    cJSON_AddStringToObject( material, "Name", m_Name );
    if( m_pShaderGroup )
        cJSON_AddStringToObject( material, "Shader", m_pShaderGroup->GetName() );
    if( m_pTextureColor )
        cJSON_AddStringToObject( material, "TexColor", m_pTextureColor->m_Filename );

    // dump animarray to disk
    char* jsonstr = cJSON_Print( root );
    cJSON_Delete( root );

    FILE* pFile;
#if MYFW_WINDOWS
    fopen_s( &pFile, filename, "wb" );
#else
    pFile = fopen( filename, "wb" );
#endif
    fprintf( pFile, "%s", jsonstr );
    fclose( pFile );

    cJSONExt_free( jsonstr );
}
#endif //MYFW_USING_WX

MaterialManager::MaterialManager()
{
    g_pPanelMemory->SetMatrialPanelCallbacks(this, MaterialManager::StaticOnLeftClick, MaterialManager::StaticOnRightClick, MaterialManager::StaticOnDrag);
}

MaterialManager::~MaterialManager()
{
    FreeAllMaterials();
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
        ((MaterialDefinition*)pNode)->Release();
    }

    while( CPPListNode* pNode = m_Materials.GetHead() )
    {
        ((MaterialDefinition*)pNode)->Release();
    }
}

void MaterialManager::LoadMaterial(const char* filename)
{
    MaterialDefinition* pMaterial = MyNew MaterialDefinition();
    m_MaterialsStillLoading.AddTail( pMaterial );

    pMaterial->m_pFile = RequestFile( filename );

#if MYFW_USING_WX
    g_pPanelMemory->AddMaterial( pMaterial, "Global", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
#endif
}

#if MYFW_USING_WX
void MaterialManager::SaveAllMaterials(bool saveunchanged)
{
    for( CPPListNode* pNode = m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->m_UnsavedChanges || saveunchanged )
        {
            pMaterial->SaveMaterial();
        }
    }
}
#endif

MaterialDefinition* MaterialManager::CreateMaterial(const char* filename, ShaderGroup* m_pShaderGroup, TextureDefinition* pTextureColor)
{
    MaterialDefinition* pMaterial = MyNew MaterialDefinition();
    m_Materials.AddTail( pMaterial );
    
    pMaterial->m_UnsavedChanges = true;
    strcpy_s( pMaterial->m_Name, MaterialDefinition::MAX_MATERIAL_NAME_LEN, filename );
    pMaterial->m_pShaderGroup = m_pShaderGroup;
    pMaterial->m_pTextureColor = pTextureColor;

#if MYFW_USING_WX
    g_pPanelMemory->AddMaterial( pMaterial, "Global", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
#endif

    return 0;
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

#if MYFW_USING_WX
void MaterialManager::OnLeftClick()
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
#if MYFW_WINDOWS
    int id = evt.GetId();
    if( id == 1000 )
    {
        g_pMaterialManager->CreateMaterial( "new mat", 0, 0 );
    }
#endif
}

void MaterialManager::OnDrag()
{
    //g_DragAndDropStruct.m_Type = DragAndDropType_MaterialDefinitionPointer;
    //g_DragAndDropStruct.m_Value = this;
}
#endif