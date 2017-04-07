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

MaterialDefinition& MaterialDefinition::operator=(const MaterialDefinition& other)
{
    MyAssert( &other != this );

    // Doesn't copy variables associated with file on disk.
    // m_UnsavedChanges
    // m_Name
    // m_pFile

    this->SetShader( other.GetShader() );
    this->SetShaderInstanced( other.GetShaderInstanced() );
    this->SetTextureColor( other.GetTextureColor() );

    this->m_BlendType = other.m_BlendType;

    this->m_ColorAmbient = other.m_ColorAmbient;
    this->m_ColorDiffuse = other.m_ColorDiffuse;
    this->m_ColorSpecular = other.m_ColorSpecular;
    this->m_Shininess = other.m_Shininess;
    this->m_UVScale = other.m_UVScale;
    this->m_UVOffset = other.m_UVOffset;

    // fully loaded flag isn't copied.
    //m_FullyLoaded

    return *this;
}

void MaterialDefinition::ImportFromFile()
{
    // TODO: replace asserts: if a shader or texture isn't found, load it.

    MyAssert( m_pFile && m_pFile->GetFileLoadStatus() == FileLoadStatus_Success );
    if( m_pFile == 0 || m_pFile->GetFileLoadStatus() != FileLoadStatus_Success )
        return;

    cJSON* jRoot = cJSON_Parse( m_pFile->GetBuffer() );

    if( jRoot == 0 )
        return;

    cJSON* jMaterial = cJSON_GetObjectItem( jRoot, "Material" );
    if( jMaterial )
    {
        cJSONExt_GetString( jMaterial, "Name", m_Name, MAX_MATERIAL_NAME_LEN );

        cJSON* jShaderString = cJSON_GetObjectItem( jMaterial, "Shader" );
        if( jShaderString )
        {
            ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByFilename( jShaderString->valuestring );
            if( pShaderGroup != 0 )
            {
                SetShader( pShaderGroup );
            }
            else
            {
                MyFileObject* pFile = g_pFileManager->RequestFile( jShaderString->valuestring );
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

        jShaderString = cJSON_GetObjectItem( jMaterial, "ShaderInstanced" );
        if( jShaderString )
        {
            ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByFilename( jShaderString->valuestring );
            if( pShaderGroup != 0 )
            {
                SetShaderInstanced( pShaderGroup );
            }
            else
            {
                MyFileObject* pFile = g_pFileManager->RequestFile( jShaderString->valuestring );
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

        cJSON* jTexColor = cJSON_GetObjectItem( jMaterial, "TexColor" );
        if( jTexColor && jTexColor->valuestring[0] != 0 )
        {
            TextureDefinition* pTexture = g_pTextureManager->CreateTexture( jTexColor->valuestring ); // adds a ref.
            MyAssert( pTexture ); // CreateTexture should find the old one if loaded or create a new one if not.
            if( pTexture )
            {
                SetTextureColor( pTexture ); // adds a reference to the texture;
            }
            pTexture->Release(); // release the ref added by CreateTexture();
        }

        ColorFloat tempcolor;
        
        cJSONExt_GetFloatArray( jMaterial, "ColorAmbient", &tempcolor.r, 4 );
        m_ColorAmbient = tempcolor.AsColorByte();

        cJSONExt_GetFloatArray( jMaterial, "ColorDiffuse", &tempcolor.r, 4 );
        m_ColorDiffuse = tempcolor.AsColorByte();

        cJSONExt_GetFloatArray( jMaterial, "ColorSpecular", &tempcolor.r, 4 );
        m_ColorSpecular = tempcolor.AsColorByte();

        cJSONExt_GetFloat( jMaterial, "Shininess", &m_Shininess );

        cJSONExt_GetInt( jMaterial, "Blend", (int*)&m_BlendType );

        cJSONExt_GetFloatArray( jMaterial, "UVScale", &m_UVScale.x, 2 );
        cJSONExt_GetFloatArray( jMaterial, "UVOffset", &m_UVOffset.x, 2 );

        m_FullyLoaded = true;
    }

    cJSON_Delete( jRoot );
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


const char* MaterialDefinition::GetMaterialDescription()
{
    if( m_pFile )
        return m_pFile->GetFullPath();

    return 0;
}

const char* MaterialDefinition::GetMaterialShortDescription()
{
    if( m_pFile )
        return m_pFile->GetFilenameWithoutExtension();

    return 0;
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

            g_pPanelWatch->AddVector2( "UVScale", &pMaterial->m_UVScale, 0, 1 );
            g_pPanelWatch->AddVector2( "UVOffset", &pMaterial->m_UVOffset, 0, 1 );

            const char* desc = "no shader";
            if( pMaterial->m_pShaderGroup && pMaterial->m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile )
                desc = pMaterial->m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->GetFilenameWithoutExtension();
            pMaterial->m_ControlID_Shader = g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, pMaterial, MaterialDefinition::StaticOnDropShader );

            desc = "no shader";
            if( pMaterial->m_pShaderGroupInstanced && pMaterial->m_pShaderGroupInstanced->GetShader( ShaderPass_Main )->m_pFile )
                desc = pMaterial->m_pShaderGroupInstanced->GetShader( ShaderPass_Main )->m_pFile->GetFilenameWithoutExtension();
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
        strcpy_s( filename, MAX_PATH, m_pFile->GetFullPath() );
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
        cJSON* jRoot = cJSON_CreateObject();

        cJSON* jMaterial = cJSON_CreateObject();
        cJSON_AddItemToObject( jRoot, "Material", jMaterial );

        cJSON_AddStringToObject( jMaterial, "Name", m_Name );
        if( m_pShaderGroup )
            cJSON_AddStringToObject( jMaterial, "Shader", m_pShaderGroup->GetFile()->GetFullPath() );
        if( m_pShaderGroupInstanced )
            cJSON_AddStringToObject( jMaterial, "ShaderInstanced", m_pShaderGroupInstanced->GetFile()->GetFullPath() );
        if( m_pTextureColor )
            cJSON_AddStringToObject( jMaterial, "TexColor", m_pTextureColor->m_Filename );

        ColorFloat tempcolor = m_ColorAmbient.AsColorFloat();
        cJSONExt_AddFloatArrayToObject( jMaterial, "ColorAmbient", &tempcolor.r, 4 );

        tempcolor = m_ColorDiffuse.AsColorFloat();
        cJSONExt_AddFloatArrayToObject( jMaterial, "ColorDiffuse", &tempcolor.r, 4 );

        tempcolor = m_ColorSpecular.AsColorFloat();
        cJSONExt_AddFloatArrayToObject( jMaterial, "ColorSpecular", &tempcolor.r, 4 );

        //cJSONExt_AddUnsignedCharArrayToObject( jMaterial, "Tint", &m_Tint.r, 4 );
        //cJSONExt_AddUnsignedCharArrayToObject( jMaterial, "SpecColor", &m_SpecColor.r, 4 );
        cJSON_AddNumberToObject( jMaterial, "Shininess", m_Shininess );

        cJSON_AddNumberToObject( jMaterial, "Blend", m_BlendType );

        cJSONExt_AddFloatArrayToObject( jMaterial, "UVScale", &m_UVScale.x, 2 );
        cJSONExt_AddFloatArrayToObject( jMaterial, "UVOffset", &m_UVOffset.x, 2 );

        // dump material json structure to disk
        jsonstr = cJSON_Print( jRoot );
        cJSON_Delete( jRoot );
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
        g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.m_ID )->m_Description = pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->GetFilenameWithoutExtension();
    }
}

void MaterialDefinition::OnDropTexture(int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );
        //MyAssert( m_pMesh );

        const char* pPath = pFile->GetFullPath();
        size_t len = strlen( pPath );
        const char* filenameext = &pPath[len-4];

        if( strcmp( filenameext, ".png" ) == 0 )
        {
            SetTextureColor( g_pTextureManager->FindTexture( pFile->GetFullPath() ) );
        }

        // update the panel so new texture name shows up.
        g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.m_ID )->m_Description = m_pTextureColor->m_Filename;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        SetTextureColor( (TextureDefinition*)g_DragAndDropStruct.m_Value );

        // update the panel so new texture name shows up.
        g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.m_ID )->m_Description = m_pTextureColor->m_Filename;
    }
}
#endif //MYFW_USING_WX
