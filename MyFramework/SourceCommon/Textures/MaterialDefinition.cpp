//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
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

const char* MaterialBlendTypeStrings[MaterialBlendType_NumTypes] =
{
    "Use Shader Setting",
    "Off",
    "On",
};

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

MaterialDefinition::MaterialDefinition(ShaderGroup* pShader, ColorByte colordiffuse)
{
    Init();

    m_pShaderGroup = pShader;
    if( m_pShaderGroup )
        m_pShaderGroup->AddRef();

    m_ColorDiffuse = colordiffuse;
}

void MaterialDefinition::Init()
{
    m_FullyLoaded = false;

    m_UnsavedChanges = false;

    m_Name[0] = 0;
    m_pFile = 0;

    m_pShaderGroup = 0;
    m_pShaderGroupInstanced = 0;
    m_pTextureColor = 0;

    m_BlendType = MaterialBlendType_UseShaderValue;

    m_ColorAmbient = ColorByte(255,255,255,255);
    m_ColorDiffuse = ColorByte(255,255,255,255);
    m_ColorSpecular = ColorByte(255,255,255,255);
    m_Shininess = 200;

    m_UVScale.Set( 1, 1 );
    m_UVOffset.Set( 0, 0 );

#if MYFW_USING_WX
    m_ControlID_Shader = -1;
    m_ControlID_ShaderInstanced = -1;
#endif
}

MaterialDefinition::~MaterialDefinition()
{
    // not all materials are in the MaterialManagers list.
    if( Prev )
        this->Remove();

#if MYFW_USING_WX
    g_pPanelMemory->RemoveMaterial( this );
#endif

    SAFE_RELEASE( m_pFile );
    SAFE_RELEASE( m_pTextureColor );
    SAFE_RELEASE( m_pShaderGroup );
    SAFE_RELEASE( m_pShaderGroupInstanced );
}

void MaterialDefinition::ImportFromFile()
{
    // TODO: replace asserts: if a shader or texture isn't found, load it.

    MyAssert( m_pFile && m_pFile->m_FileLoadStatus == FileLoadStatus_Success );
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
            ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByFilename( shaderstringobj->valuestring );
            if( pShaderGroup != 0 )
            {
                SetShader( pShaderGroup );
            }
            else
            {
                MyFileObject* pFile = g_pFileManager->RequestFile( shaderstringobj->valuestring );
                if( pFile->IsA( "MyFileShader" ) )
                {
                    MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pFile;
                    pShaderGroup = MyNew ShaderGroup( pShaderFile );
                    SetShader( pShaderGroup );
                    pShaderGroup->Release();
                }
                else
                {
                    MyAssert( false );
                }
                pFile->Release();
            }
        }

        shaderstringobj = cJSON_GetObjectItem( material, "ShaderInstanced" );
        if( shaderstringobj )
        {
            ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByFilename( shaderstringobj->valuestring );
            if( pShaderGroup != 0 )
            {
                SetShaderInstanced( pShaderGroup );
            }
            else
            {
                MyFileObject* pFile = g_pFileManager->RequestFile( shaderstringobj->valuestring );
                if( pFile->IsA( "MyFileShader" ) )
                {
                    MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pFile;
                    pShaderGroup = MyNew ShaderGroup( pShaderFile );
                    SetShaderInstanced( pShaderGroup );
                    pShaderGroup->Release();
                }
                else
                {
                    MyAssert( false );
                }
                pFile->Release();
            }
        }

        cJSON* texcolorstringobj = cJSON_GetObjectItem( material, "TexColor" );
        if( texcolorstringobj && texcolorstringobj->valuestring[0] != 0 )
        {
            TextureDefinition* pTexture = g_pTextureManager->CreateTexture( texcolorstringobj->valuestring ); // adds a ref.
            MyAssert( pTexture ); // CreateTexture should find the old one if loaded or create a new one if not.
            if( pTexture )
            {
                SetTextureColor( pTexture ); // adds a reference to the texture;
            }
            pTexture->Release(); // release the ref added by CreateTexture();
        }

        ColorFloat tempcolor;
        
        cJSONExt_GetFloatArray( material, "ColorAmbient", &tempcolor.r, 4 );
        m_ColorAmbient = tempcolor.AsColorByte();

        cJSONExt_GetFloatArray( material, "ColorDiffuse", &tempcolor.r, 4 );
        m_ColorDiffuse = tempcolor.AsColorByte();

        cJSONExt_GetFloatArray( material, "ColorSpecular", &tempcolor.r, 4 );
        m_ColorSpecular = tempcolor.AsColorByte();

        cJSONExt_GetFloat( material, "Shininess", &m_Shininess );

        cJSONExt_GetInt( material, "Blend", (int*)&m_BlendType );

        m_FullyLoaded = true;
    }

    cJSON_Delete( root );
}

void MaterialDefinition::MoveAssociatedFilesToFrontOfFileList()
{
    g_pFileManager->MoveFileToFrontOfFileLoadedList( m_pFile );

    if( m_pShaderGroup )
        g_pFileManager->MoveFileToFrontOfFileLoadedList( m_pShaderGroup->GetFile() );

    if( m_pShaderGroupInstanced )
        g_pFileManager->MoveFileToFrontOfFileLoadedList( m_pShaderGroupInstanced->GetFile() );

    if( m_pTextureColor )
        g_pFileManager->MoveFileToFrontOfFileLoadedList( m_pTextureColor->m_pFile );
}

void MaterialDefinition::SetName(const char* name)
{
    MyAssert( name );

    if( strcmp( m_Name, name ) == 0 ) // name hasn't changed.
        return;

    //size_t len = strlen( name );
    
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

void MaterialDefinition::SetShaderInstanced(ShaderGroup* pShader)
{
    if( pShader )
        pShader->AddRef();
    SAFE_RELEASE( m_pShaderGroupInstanced );
    m_pShaderGroupInstanced = pShader;
}

void MaterialDefinition::SetTextureColor(TextureDefinition* pTexture)
{
    if( pTexture )
        pTexture->AddRef();
    SAFE_RELEASE( m_pTextureColor );
    m_pTextureColor = pTexture;
}

bool MaterialDefinition::IsTransparent(BaseShader* pShader)
{
    if( m_BlendType == MaterialBlendType_On )
        return true;

    // check the shader
    if( m_BlendType == MaterialBlendType_UseShaderValue )
        return pShader->m_BlendType == MaterialBlendType_On ? true : false;

    return false;
}

bool MaterialDefinition::IsTransparent()
{
    // if shader from any pass is opaque, consider entire object opaque.
    if( m_pShaderGroup )
    {
        //for( int i=0; i<ShaderPass_NumTypes; i++ )
        int i = ShaderPass_Main;
        {
            BaseShader* pShader = m_pShaderGroup->GetShader( (ShaderPassTypes)i );
            if( pShader )
            {
                if( IsTransparent( pShader ) == false )
                    return false;
            }
        }
    }

    return true;
}

#if MYFW_USING_WX
void MaterialDefinition::OnLeftClick(unsigned int count)
{
}

void MaterialDefinition::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );
    
    menu.Append( RightClick_ViewInWatchWindow, "View in watch window" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&MaterialDefinition::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void MaterialDefinition::OnPopupClick(wxEvent &evt)
{
    MaterialDefinition* pMaterial = (MaterialDefinition*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    int id = evt.GetId();
    switch( id )
    {
    case RightClick_ViewInWatchWindow:
        {
            g_pPanelWatch->ClearAllVariables();

            g_pPanelWatch->AddSpace( pMaterial->m_Name );

            g_pPanelWatch->AddEnum( "Blend", (int*)&pMaterial->m_BlendType, MaterialBlendType_NumTypes, MaterialBlendTypeStrings );

            const char* desc = "no shader";
            if( pMaterial->m_pShaderGroup && pMaterial->m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile )
                desc = pMaterial->m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
            pMaterial->m_ControlID_Shader = g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, pMaterial, MaterialDefinition::StaticOnDropShader );

            desc = "no shader";
            if( pMaterial->m_pShaderGroupInstanced && pMaterial->m_pShaderGroupInstanced->GetShader( ShaderPass_Main )->m_pFile )
                desc = pMaterial->m_pShaderGroupInstanced->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
            pMaterial->m_ControlID_ShaderInstanced = g_pPanelWatch->AddPointerWithDescription( "Shader Instanced", 0, desc, pMaterial, MaterialDefinition::StaticOnDropShader );

            desc = "no color texture";
            if( pMaterial->m_pTextureColor )
                desc = pMaterial->m_pTextureColor->m_Filename;
            g_pPanelWatch->AddPointerWithDescription( "Color Texture", 0, desc, pMaterial, MaterialDefinition::StaticOnDropTexture );

            g_pPanelWatch->AddColorByte( "Color-Ambient", &pMaterial->m_ColorAmbient, 0, 255 );
            g_pPanelWatch->AddColorByte( "Color-Diffuse", &pMaterial->m_ColorDiffuse, 0, 255 );
            g_pPanelWatch->AddColorByte( "Color-Specular", &pMaterial->m_ColorSpecular, 0, 255 );

            g_pPanelWatch->AddFloat( "Shininess", &pMaterial->m_Shininess, 1, 300 );
        }
        break;
    }
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

void MaterialDefinition::SaveMaterial(const char* relativepath)
{
    if( m_Name[0] == 0 )
        return;

    char filename[MAX_PATH];

    if( m_pFile != 0 )
    {
        // if a file exists, use the existing file's fullpath
        strcpy_s( filename, MAX_PATH, m_pFile->m_FullPath );
    }
    else
    {
        // if a file doesn't exist, create the filename out of parts.
        // TODO: move most of this block into generic system code.
        MyAssert( relativepath != 0 );

        char workingdir[MAX_PATH];
#if MYFW_WINDOWS
        _getcwd( workingdir, MAX_PATH * sizeof(char) );
#else
        getcwd( workingdir, MAX_PATH * sizeof(char) );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/", workingdir, relativepath );
#if MYFW_WINDOWS
        CreateDirectoryA( filename, 0 );
#else
        MyAssert( false );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/%s.mymaterial", workingdir, relativepath, m_Name );

        // this is a new file, check for filename conflict
        {
            unsigned int count = 0;
            char newname[MAX_MATERIAL_NAME_LEN];
            strcpy_s( newname, MAX_MATERIAL_NAME_LEN, m_Name );
            while( g_pFileManager->DoesFileExist( filename ) == true )
            {
                count++;

                sprintf_s( newname, "%s(%d)", m_Name, count );
                sprintf_s( filename, MAX_PATH, "%s/%s/%s.mymaterial", workingdir, relativepath, newname );
            }
            strcpy_s( m_Name, MAX_MATERIAL_NAME_LEN, newname );
        }
    }

    // Create the json string to save into the material file
    char* jsonstr = 0;
    {
        cJSON* root = cJSON_CreateObject();

        cJSON* material = cJSON_CreateObject();
        cJSON_AddItemToObject( root, "Material", material );

        cJSON_AddStringToObject( material, "Name", m_Name );
        if( m_pShaderGroup )
            cJSON_AddStringToObject( material, "Shader", m_pShaderGroup->GetFile()->m_FullPath );
        if( m_pShaderGroupInstanced )
            cJSON_AddStringToObject( material, "ShaderInstanced", m_pShaderGroupInstanced->GetFile()->m_FullPath );
        if( m_pTextureColor )
            cJSON_AddStringToObject( material, "TexColor", m_pTextureColor->m_Filename );

        ColorFloat tempcolor = m_ColorAmbient.AsColorFloat();
        cJSONExt_AddFloatArrayToObject( material, "ColorAmbient", &tempcolor.r, 4 );

        tempcolor = m_ColorDiffuse.AsColorFloat();
        cJSONExt_AddFloatArrayToObject( material, "ColorDiffuse", &tempcolor.r, 4 );

        tempcolor = m_ColorSpecular.AsColorFloat();
        cJSONExt_AddFloatArrayToObject( material, "ColorSpecular", &tempcolor.r, 4 );

        //cJSONExt_AddUnsignedCharArrayToObject( material, "Tint", &m_Tint.r, 4 );
        //cJSONExt_AddUnsignedCharArrayToObject( material, "SpecColor", &m_SpecColor.r, 4 );
        cJSON_AddNumberToObject( material, "Shininess", m_Shininess );

        cJSON_AddNumberToObject( material, "Blend", m_BlendType );

        // dump material json structure to disk
        jsonstr = cJSON_Print( root );
        cJSON_Delete( root );
    }

    if( jsonstr != 0 )
    {
        FILE* pFile = 0;
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

        // if the file managed to save, request it.
        if( m_pFile == 0 )
        {
            sprintf_s( filename, MAX_PATH, "%s/%s.mymaterial", relativepath, m_Name );
            m_pFile = g_pFileManager->RequestFile( filename );
        }
    }
}

void MaterialDefinition::OnDropShader(int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)g_DragAndDropStruct.m_Value;
        MyAssert( pShaderGroup );
        //MyAssert( m_pMesh );

        if( controlid == m_ControlID_Shader )
        {
            SetShader( pShaderGroup );
        }
        else if( controlid == m_ControlID_ShaderInstanced )
        {
            SetShaderInstanced( pShaderGroup );
        }
        else
        {
            MyAssert( false );
        }

        // update the panel so new Shader name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
    }
}

void MaterialDefinition::OnDropTexture(int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );
        //MyAssert( m_pMesh );

        size_t len = strlen( pFile->m_FullPath );
        const char* filenameext = &pFile->m_FullPath[len-4];

        if( strcmp( filenameext, ".png" ) == 0 )
        {
            SetTextureColor( g_pTextureManager->FindTexture( pFile->m_FullPath ) );
        }

        // update the panel so new texture name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pTextureColor->m_Filename;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        SetTextureColor( (TextureDefinition*)g_DragAndDropStruct.m_Value );

        // update the panel so new texture name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pTextureColor->m_Filename;
    }
}
#endif //MYFW_USING_WX
