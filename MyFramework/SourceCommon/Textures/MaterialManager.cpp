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
    Init();
}

MaterialDefinition::MaterialDefinition(ShaderGroup* pShader)
{
    Init();

    m_pShaderGroup = pShader;
    if( m_pShaderGroup )
        m_pShaderGroup->AddRef();
}

MaterialDefinition::MaterialDefinition(ShaderGroup* pShader, ColorByte tint)
{
    Init();

    m_pShaderGroup = pShader;
    if( m_pShaderGroup )
        m_pShaderGroup->AddRef();

    m_Tint = tint;
}

void MaterialDefinition::Init()
{
    m_FullyLoaded = false;

    m_UnsavedChanges = false;

    m_Name[0] = 0;
    m_pFile = 0;

    m_pShaderGroup = 0;
    m_pTextureColor = 0;
    m_Tint = ColorByte(255,255,255,255);
    m_SpecColor = ColorByte(255,255,255,255);
    m_Shininess = 200;
}


MaterialDefinition::~MaterialDefinition()
{
    // not all materials are in the MaterialManagers list.
    if( Prev )
        this->Remove();

#if MYFW_USING_WX
    g_pPanelMemory->RemoveMaterial( this );
#endif

    if( m_pFile )
    {
        g_pFileManager->FreeFile( m_pFile );
    }

    SAFE_RELEASE( m_pTextureColor );
    if( m_pFile != 0 )
        int bp = 1;
    SAFE_RELEASE( m_pShaderGroup );
}

void MaterialDefinition::ImportFromFile()
{
    // TODO: replace asserts: if a shader or texture isn't found, load it.

    assert( m_pFile && m_pFile->m_FileLoadStatus == FileLoadStatus_Success );
    if( m_pFile == 0 || m_pFile->m_FileLoadStatus != FileLoadStatus_Success )
        return;

    cJSON* root = cJSON_Parse( m_pFile->m_pBuffer );

    cJSON* material = cJSON_GetObjectItem( root, "Material" );
    if( material )
    {
        cJSONExt_GetString( material, "Name", m_Name, MAX_MATERIAL_NAME_LEN );

        cJSON* shaderstringobj = cJSON_GetObjectItem( material, "Shader" );
        if( shaderstringobj )
        {
            ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByName( shaderstringobj->valuestring );
            assert( pShaderGroup ); // fix

            SetShader( pShaderGroup );
        }

        cJSON* texcolorstringobj = cJSON_GetObjectItem( material, "TexColor" );
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

        cJSONExt_GetUnsignedCharArray( material, "Tint", &m_Tint.r, 4 );
        cJSONExt_GetUnsignedCharArray( material, "SpecColor", &m_SpecColor.r, 4 );
        cJSONExt_GetFloat( material, "Shininess", &m_Shininess );

        m_FullyLoaded = true;
    }

    cJSON_Delete( root );
}

void MaterialDefinition::SetName(const char* name)
{
    assert( name );

    if( strcmp( m_Name, name ) == 0 ) // name hasn't changed.
        return;

    size_t len = strlen( name );
    
    strcpy_s( m_Name, MAX_MATERIAL_NAME_LEN, name );

    if( m_pFile )
    {
        m_pFile->Rename( name );
        // TODO: rename the file on disk.
    }

#if MYFW_USING_WX
    if( g_pPanelMemory )
    {
        g_pPanelMemory->RenameObject( g_pPanelMemory->m_pTree_Materials, this, m_Name );
    }
#endif //MYFW_USING_WX
}

void MaterialDefinition::SetShader(ShaderGroup* pShader)
{
    if( pShader )
        pShader->AddRef();
    SAFE_RELEASE( m_pShaderGroup );
    m_pShaderGroup = pShader;
}

void MaterialDefinition::SetTextureColor(TextureDefinition* pTexture)
{
    if( pTexture )
        pTexture->AddRef();
    SAFE_RELEASE( m_pTextureColor );
    m_pTextureColor = pTexture;
}

#if MYFW_USING_WX
void MaterialDefinition::OnLeftClick()
{
    g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->AddSpace( this->m_Name );

    const char* desc = "no shader";
    if( m_pShaderGroup && m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile )
        desc = m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
    g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, this, MaterialDefinition::StaticOnDropShader );

    desc = "no color texture";
    if( m_pTextureColor )
        desc = m_pTextureColor->m_Filename;
    g_pPanelWatch->AddPointerWithDescription( "Tex Color", 0, desc, this, MaterialDefinition::StaticOnDropTexture );

    //g_pPanelWatch->AddVector3( "Pos", &m_Position, -1.0f, 1.0f, this, ComponentTransform::StaticOnValueChanged );
    //g_pPanelWatch->AddVector3( "Scale", &m_Scale, 0.0f, 10.0f, this, ComponentTransform::StaticOnValueChanged );
    //g_pPanelWatch->AddVector3( "Rot", &m_Rotation, 0, 360, this, ComponentTransform::StaticOnValueChanged );
    //ColorByte m_Tint;
    //ColorByte m_SpecColor;

    g_pPanelWatch->AddFloat( "Shininess", &m_Shininess, 1, 300 );
}

void MaterialDefinition::OnRightClick()
{
}

void MaterialDefinition::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_MaterialDefinitionPointer;
    g_DragAndDropStruct.m_Value = this;
}

void MaterialDefinition::OnLabelEdit(wxString newlabel)
{
    size_t len = newlabel.length();
    if( len > 0 )
    {
        SetName( newlabel );
    }
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
    sprintf_s( filename, MAX_PATH, "%s/Data/Materials/", workingdir );
    CreateDirectoryA( filename, 0 );
    sprintf_s( filename, MAX_PATH, "%s/Data/Materials/%s.mymaterial", workingdir, m_Name );

    cJSON* root = cJSON_CreateObject();

    cJSON* material = cJSON_CreateObject();
    cJSON_AddItemToObject( root, "Material", material );

    cJSON_AddStringToObject( material, "Name", m_Name );
    if( m_pShaderGroup )
        cJSON_AddStringToObject( material, "Shader", m_pShaderGroup->GetName() );
    if( m_pTextureColor )
        cJSON_AddStringToObject( material, "TexColor", m_pTextureColor->m_Filename );

    cJSONExt_AddUnsignedCharArrayToObject( material, "Tint", &m_Tint.r, 4 );
    cJSONExt_AddUnsignedCharArrayToObject( material, "SpecColor", &m_SpecColor.r, 4 );
    cJSON_AddNumberToObject( material, "Shininess", m_Shininess );

    // dump animarray to disk
    char* jsonstr = cJSON_Print( root );
    cJSON_Delete( root );

    FILE* pFile;
#if MYFW_WINDOWS
    fopen_s( &pFile, filename, "wb" );
#else
    pFile = fopen( filename, "wb" );
#endif
    if( pFile )
    {
        fprintf( pFile, "%s", jsonstr );
        fclose( pFile );
    }

    cJSONExt_free( jsonstr );

    if( m_pFile == 0 )
    {
        sprintf_s( filename, MAX_PATH, "Data/Materials/%s.mymaterial", m_Name );
        m_pFile = g_pFileManager->RequestFile( filename );
    }
}

void MaterialDefinition::OnDropShader()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)g_DragAndDropStruct.m_Value;
        assert( pShaderGroup );
        //assert( m_pMesh );

        SetShader( pShaderGroup );

        // update the panel so new Shader name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
    }
}

void MaterialDefinition::OnDropTexture()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        assert( pFile );
        //assert( m_pMesh );

        size_t len = strlen( pFile->m_FullPath );
        const char* filenameext = &pFile->m_FullPath[len-4];

        if( strcmp( filenameext, ".png" ) == 0 )
        {
            TextureDefinition* pOldTexture = m_pTextureColor;
            m_pTextureColor = g_pTextureManager->FindTexture( pFile->m_FullPath );
            SAFE_RELEASE( pOldTexture );
        }

        // update the panel so new Shader name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pTextureColor->m_Filename;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        TextureDefinition* pOldTexture = m_pTextureColor;
        m_pTextureColor = (TextureDefinition*)g_DragAndDropStruct.m_Value;
        SAFE_RELEASE( pOldTexture );

        // update the panel so new Shader name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pTextureColor->m_Filename;
    }
}
#endif //MYFW_USING_WX

MaterialManager::MaterialManager()
{
#if MYFW_USING_WX
    g_pPanelMemory->SetMaterialPanelCallbacks(this, MaterialManager::StaticOnLeftClick, MaterialManager::StaticOnRightClick, MaterialManager::StaticOnDrag);
#endif
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

#if MYFW_USING_WX
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
    g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, this, MaterialDefinition::StaticOnLabelEdit );
#endif
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
    pMaterial->m_UnsavedChanges = true;
    strcpy_s( pMaterial->m_Name, MaterialDefinition::MAX_MATERIAL_NAME_LEN, name );

#if MYFW_USING_WX
    g_pPanelMemory->AddMaterial( pMaterial, "Global", pMaterial->m_Name, MaterialDefinition::StaticOnLeftClick, MaterialDefinition::StaticOnRightClick, MaterialDefinition::StaticOnDrag );
    g_pPanelMemory->SetLabelEditFunction( g_pPanelMemory->m_pTree_Materials, this, MaterialDefinition::StaticOnLabelEdit );
#endif

    return 0;
}

MaterialDefinition* MaterialManager::CreateMaterial(MyFileObject* pFile)
{
    assert( pFile );

    MaterialDefinition* pMaterial = MyNew MaterialDefinition();
    m_MaterialsStillLoading.AddTail( pMaterial );
    
    pMaterial->m_FullyLoaded = false;
    pMaterial->m_UnsavedChanges = false;
    pMaterial->m_pFile = pFile;
    pMaterial->m_pFile->AddRef();

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

MaterialDefinition* MaterialManager::FindMaterialByFilename(const char* filename)
{
    for( CPPListNode* pNode = m_MaterialsStillLoading.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( strcmp( pMaterial->m_pFile->m_FullPath, filename ) == 0 )
            return pMaterial;
    }

    for( CPPListNode* pNode = m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->m_pFile->m_FullPath == filename )
            return pMaterial;
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
        g_pMaterialManager->CreateMaterial( "new" );
    }
#endif
}

void MaterialManager::OnDrag()
{
    //g_DragAndDropStruct.m_Type = DragAndDropType_MaterialDefinitionPointer;
    //g_DragAndDropStruct.m_Value = this;
}
#endif
