//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
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

void ExposedUniformValue::SetToInitialValue(ExposedUniformType type)
{
    switch( type )
    {
    case ExposedUniformType_Float:
        m_Float = 0;
        break;

    case ExposedUniformType_Vec2:
        ((Vector2*)&m_Vec2)->Set( 0, 0 );
        break;

    case ExposedUniformType_Vec3:
        ((Vector3*)&m_Vec3)->Set( 0, 0, 0 );
        break;

    case ExposedUniformType_Vec4:
        ((Vector4*)&m_Vec4)->Set( 0, 0, 0, 1 );
        break;

    case ExposedUniformType_ColorByte:
        ((ColorByte*)&m_ColorByte)->Set( 0, 0, 0, 255 );
        break;

    case ExposedUniformType_Sampler2D:
        m_pTexture = nullptr;
        break;

    case ExposedUniformType_NotSet:
    default:
        MyAssert( false );
        break;
    }
}

MaterialDefinition::MaterialDefinition()
{
    Init();
}

MaterialDefinition::MaterialDefinition(ShaderGroup* pShader)
{
    Init();

    SetShader( pShader );
}

MaterialDefinition::MaterialDefinition(ShaderGroup* pShader, ColorByte colordiffuse)
{
    Init();

    SetShader( pShader );

    m_ColorDiffuse = colordiffuse;

    m_UnsavedChanges = false;
}

//============================================================================================================================
// Protected/Internal methods.
//============================================================================================================================
void MaterialDefinition::SetFile(MyFileObject* pFile)
{
    // This method should only be used by the Material Manager when initially creating a material from an pre-created file object.
    // So, make sure this material doesn't already have a file pointer.
    MyAssert( m_pFile == nullptr );
    MyAssert( m_MaterialFileIsLoaded == false );

    m_pFile = pFile;
    pFile->AddRef();

    m_MaterialFileIsLoaded = true;
    m_UnsavedChanges = true;
    strcpy_s( m_Name, MAX_MATERIAL_NAME_LEN, pFile->GetFilenameWithoutExtension() );
}

//============================================================================================================================
// Public methods.
//============================================================================================================================
void MaterialDefinition::Init()
{
    m_MaterialFileIsLoaded = false;

    m_UnsavedChanges = false;

    m_Name[0] = '\0';
    m_pFile = nullptr;

    m_pShaderGroup = nullptr;
    m_pShaderGroupInstanced = nullptr;
    m_pTextureColor = nullptr;

    m_BlendType = MyRE::MaterialBlendType_UseShaderValue;

    m_ColorAmbient = ColorByte(255,255,255,255);
    m_ColorDiffuse = ColorByte(255,255,255,255);
    m_ColorSpecular = ColorByte(255,255,255,255);
    m_Shininess = 200;

    m_UVScale.Set( 1, 1 );
    m_UVOffset.Set( 0, 0 );

#if MYFW_EDITOR
    m_PreviewType = PreviewType_Sphere;

    for( unsigned int i=0; i<MyFileObjectShader::MAX_EXPOSED_UNIFORMS; i++ )
    {
        m_UniformValues[i].m_Name = "";
        m_UniformValues[i].m_Type = ExposedUniformType_NotSet;
    }
#endif //MYFW_EDITOR
}

MaterialDefinition::~MaterialDefinition()
{
    // Not all materials are in the MaterialManagers list.
    if( Prev )
        this->Remove();

    // Release the exposed uniform texturedef pointers.
    if( m_pShaderGroup && m_pShaderGroup->GetFile() )
    {
        MyFileObjectShader* pShaderFile = m_pShaderGroup->GetFile();

        for( unsigned int i=0; i<pShaderFile->m_NumExposedUniforms; i++ )
        {
            if( pShaderFile->m_ExposedUniforms[i].m_Type == ExposedUniformType_Sampler2D )
            {
                SAFE_RELEASE( m_UniformValues[i].m_pTexture );
            }
        }
    }

    SetShader( nullptr );
    SetShaderInstanced( nullptr );

    // Release the file after setting shader to nullptr, so SetShader() can unregister the finished loading callback.
    SAFE_RELEASE( m_pFile );
    SAFE_RELEASE( m_pTextureColor );
}

MaterialDefinition& MaterialDefinition::operator=(const MaterialDefinition& other)
{
    MyAssert( &other != this );

    // Don't copy variables associated with file on disk.
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

    // Fully loaded flag isn't copied.
    //m_FullyLoaded

    // TODO: Copy the exposed uniform values.

#if MYFW_EDITOR
    this->m_PreviewType = other.m_PreviewType;
#endif

    return *this;
}

void MaterialDefinition::ImportFromFile()
{
    // TODO: Replace asserts: if a shader or texture isn't found, load it.

    MyAssert( m_pFile && m_pFile->GetFileLoadStatus() == FileLoadStatus_Success );
    if( m_pFile == nullptr || m_pFile->GetFileLoadStatus() != FileLoadStatus_Success )
        return;

    cJSON* jRoot = cJSON_Parse( m_pFile->GetBuffer() );

    if( jRoot == nullptr )
        return;

    cJSON* jMaterial = cJSON_GetObjectItem( jRoot, "Material" );
    if( jMaterial )
    {
        cJSONExt_GetString( jMaterial, "Name", m_Name, MAX_MATERIAL_NAME_LEN );

        cJSON* jShaderString = cJSON_GetObjectItem( jMaterial, "Shader" );
        if( jShaderString )
        {
            ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByFilename( jShaderString->valuestring );
            if( pShaderGroup != nullptr )
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
            if( pShaderGroup != nullptr )
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
        if( jTexColor && jTexColor->valuestring[0] != '\0' )
        {
            TextureDefinition* pTexture = g_pTextureManager->CreateTexture( jTexColor->valuestring ); // Adds a ref.
            MyAssert( pTexture ); // CreateTexture should find the old one if loaded or create a new one if not.
            if( pTexture )
            {
                SetTextureColor( pTexture ); // Adds a reference to the texture.
            }
            pTexture->Release(); // Release the ref added by CreateTexture().
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

#if MYFW_EDITOR
        cJSONExt_GetInt( jMaterial, "PreviewType", (int*)&m_PreviewType );
#endif //MYFW_EDITOR

        if( m_pShaderGroup && m_pShaderGroup->GetFile() && m_pShaderGroup->GetFile()->IsFinishedLoading() )
        {
            ImportExposedUniformValues( jMaterial );
        }

        m_MaterialFileIsLoaded = true;
    }

    cJSON_Delete( jRoot );

    m_UnsavedChanges = false;
}

void MaterialDefinition::MoveAssociatedFilesToFrontOfFileList()
{
    g_pFileManager->MoveFileToFrontOfFileLoadedList( m_pFile );

    if( m_pShaderGroup )
        g_pFileManager->MoveFileToFrontOfFileLoadedList( m_pShaderGroup->GetFile() );

    if( m_pShaderGroupInstanced )
        g_pFileManager->MoveFileToFrontOfFileLoadedList( m_pShaderGroupInstanced->GetFile() );

    if( m_pTextureColor )
        g_pFileManager->MoveFileToFrontOfFileLoadedList( m_pTextureColor->GetFile() );
}

void MaterialDefinition::SetName(const char* name)
{
    MyAssert( name );

    if( strcmp( m_Name, name ) == 0 ) // Name hasn't changed.
        return;

    strcpy_s( m_Name, MAX_MATERIAL_NAME_LEN, name );

    if( m_pFile )
    {
        // Rename the file on disk and force a save to update the name in the material file.
        m_pFile->Rename( name );
#if MYFW_EDITOR
        SaveMaterial( nullptr );
#endif //MYFW_EDITOR
    }
}

bool MaterialDefinition::IsShaderLoaded()
{
    if( m_pFile && m_MaterialFileIsLoaded == false )
    {
        return false;
    }

    if( m_pShaderGroup && m_pShaderGroup->GetFile() )
    {
        if( m_pShaderGroup->GetFile()->IsFinishedLoading() == false )
            return false;
    }

    if( m_pShaderGroupInstanced && m_pShaderGroupInstanced->GetFile() )
    {
        if( m_pShaderGroupInstanced->GetFile()->IsFinishedLoading() == false )
            return false;
    }

    return true;
}

bool MaterialDefinition::IsFullyLoaded()
{
    if( IsShaderLoaded() == false )
    {
        return false;
    }

    if( m_pTextureColor && m_pTextureColor->GetFile() )
    {
        if( m_pTextureColor->GetFile()->IsFinishedLoading() == false )
            return false;
    }

    return true;
}

const char* MaterialDefinition::GetMaterialDescription()
{
    if( m_pFile )
        return m_pFile->GetFullPath();

    return nullptr;
}

const char* MaterialDefinition::GetMaterialShortDescription()
{
    if( m_pFile )
        return m_pFile->GetFilenameWithoutExtension();

    return nullptr;
}

void MaterialDefinition::SetShader(ShaderGroup* pShader)
{
    if( pShader == m_pShaderGroup )
        return;

    m_UnsavedChanges = true;

    if( pShader )
        pShader->AddRef();
    
    // Free the current shader and unregister it's file's "finished loading" callback if one was set.
    if( m_pShaderGroup && m_pShaderGroup->GetFile() )
        m_pShaderGroup->GetFile()->UnregisterFileFinishedLoadingCallback( this );
    SAFE_RELEASE( m_pShaderGroup );

    m_pShaderGroup = pShader;

    if( m_pShaderGroup && m_pShaderGroup->GetFile() )
    {
        if( m_pShaderGroup->GetFile()->IsFinishedLoading() == false )
        {
            m_pShaderGroup->GetFile()->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoading );
        }
        else
        {
            // In editor builds, always register finished loading callback to know if the file was reloaded due to changes.
#if MYFW_EDITOR
            m_pShaderGroup->GetFile()->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoading );
#endif

            InitializeExposedUniformValues( false );

            if( m_pFile && m_pFile->GetFileLoadStatus() == FileLoadStatus_Success )
            {
                ImportFromFile();
            }
        }
    }
}

void MaterialDefinition::OnFileFinishedLoading(MyFileObject* pFile) // StaticOnFileFinishedLoading
{
    // In editor builds, keep the finished loading callback registered,
    //     so we can reset exposed uniforms if shader file is reloaded due to changes.
#if !MYFW_EDITOR
    // Unregister this callback.
    pFile->UnregisterFileFinishedLoadingCallback( this );
#endif

    // Shader file finished loading, so set all exposed uniforms to 0 and reimport our material (if loaded),
    //     which will reimport saved exposed uniform values.
    InitializeExposedUniformValues( true );
}

void MaterialDefinition::InitializeExposedUniformValues(bool maintainexistingvalues)
{
#if MYFW_EDITOR
    static ExposedUniformValue g_PreviousUniformValues[MyFileObjectShader::MAX_EXPOSED_UNIFORMS];

    // Backup the old values, then restore them below.
    for( unsigned int i=0; i<MyFileObjectShader::MAX_EXPOSED_UNIFORMS; i++ )
    {
        g_PreviousUniformValues[i] = m_UniformValues[i];
    }
#endif

    if( m_pShaderGroup )
    {
        MyFileObjectShader* pShaderFile = m_pShaderGroup->GetFile();

        if( pShaderFile->m_ScannedForExposedUniforms == false )
        {
            pShaderFile->ParseAndCleanupExposedUniforms();
        }

        if( pShaderFile )
        {
            for( unsigned int i=0; i<pShaderFile->m_NumExposedUniforms; i++ )
            {
                // Set the uniforms to it's initial value (generally zero).
                m_UniformValues[i].SetToInitialValue( pShaderFile->m_ExposedUniforms[i].m_Type );
            }

#if MYFW_EDITOR
            // Copy the names of all the uniforms first, will be used to search for values.
            for( unsigned int i=0; i<MyFileObjectShader::MAX_EXPOSED_UNIFORMS; i++ )
            {
                if( i < pShaderFile->m_NumExposedUniforms )
                    m_UniformValues[i].m_Name = pShaderFile->m_ExposedUniforms[i].m_Name;
                else
                    m_UniformValues[i].m_Name = "";
            }

            // Restore uniform values backed up above.
            for( unsigned int j=0; j<MyFileObjectShader::MAX_EXPOSED_UNIFORMS; j++ )
            {
                unsigned int i = MyFileObjectShader::MAX_EXPOSED_UNIFORMS;

                // Search for uniform in the old list, if found, grab it's old value.
                if( j < pShaderFile->m_NumExposedUniforms )
                {
                    for( i=0; i<MyFileObjectShader::MAX_EXPOSED_UNIFORMS; i++ )
                    {
                        MyAssert( m_UniformValues[j].m_Name != "" );

                        if( m_UniformValues[j].m_Name == g_PreviousUniformValues[i].m_Name )
                        {
                            m_UniformValues[j] = g_PreviousUniformValues[i];
                            break;
                        }
                    }
                }

                // If the uniform wasn't found in the old list, zero out the entry.
                if( i == MyFileObjectShader::MAX_EXPOSED_UNIFORMS )
                {
                    // If the type we have stored is a sampler, release the texture.
                    if( m_UniformValues[j].m_Type == ExposedUniformType_Sampler2D )
                    {
                        SAFE_RELEASE( g_PreviousUniformValues[j].m_pTexture );
                    }

                    if( j < pShaderFile->m_NumExposedUniforms )
                    {
                        m_UniformValues[j].SetToInitialValue( pShaderFile->m_ExposedUniforms[j].m_Type );
                    }
                }

                // Copy the new type into our array for the next time the file is reloaded.
                m_UniformValues[j].m_Type = pShaderFile->m_ExposedUniforms[j].m_Type;
            }
#endif
        }
    }
}

void MaterialDefinition::ImportExposedUniformValues(cJSON* jMaterial)
{
    if( m_pShaderGroup )
    {
        MyFileObjectShader* pShaderFile = m_pShaderGroup->GetFile();

        if( pShaderFile->m_ScannedForExposedUniforms == false )
        {
            pShaderFile->ParseAndCleanupExposedUniforms();
        }

        if( pShaderFile )
        {
            for( unsigned int i=0; i<pShaderFile->m_NumExposedUniforms; i++ )
            {
                ExposedUniformInfo* pInfo = &pShaderFile->m_ExposedUniforms[i];
                switch( pInfo->m_Type )
                {
                case ExposedUniformType_Float:
                    cJSONExt_GetFloat( jMaterial, pInfo->m_Name, &m_UniformValues[i].m_Float );
                    break;

                case ExposedUniformType_Vec2:
                    cJSONExt_GetFloatArray( jMaterial, pInfo->m_Name, m_UniformValues[i].m_Vec2, 2 );
                    break;

                case ExposedUniformType_Vec3:
                    cJSONExt_GetFloatArray( jMaterial, pInfo->m_Name, m_UniformValues[i].m_Vec3, 3 );
                    break;

                case ExposedUniformType_Vec4:
                    cJSONExt_GetFloatArray( jMaterial, pInfo->m_Name, m_UniformValues[i].m_Vec4, 4 );
                    break;

                case ExposedUniformType_ColorByte:
                    cJSONExt_GetUnsignedCharArray( jMaterial, pInfo->m_Name, m_UniformValues[i].m_ColorByte, 4 );
                    break;

                case ExposedUniformType_Sampler2D:
                    {
                        cJSON* obj = cJSON_GetObjectItem( jMaterial, pInfo->m_Name );
                        if( obj )
                        {
                            char* filename = obj->valuestring;

                            if( filename != nullptr )
                            {
                                m_UniformValues[i].m_pTexture = g_pTextureManager->FindTexture( filename );
                                m_UniformValues[i].m_pTexture->AddRef();
                            }
                        }
                    }
                    break;

                case ExposedUniformType_NotSet:
                default:
                    MyAssert( false );
                    break;
                }
            }
        }
    }
}

void MaterialDefinition::ExportExposedUniformValues(cJSON* jMaterial)
{
    if( m_pShaderGroup )
    {
        MyFileObjectShader* pShaderFile = m_pShaderGroup->GetFile();

        if( pShaderFile->m_ScannedForExposedUniforms == false )
        {
            pShaderFile->ParseAndCleanupExposedUniforms();
        }

        if( pShaderFile )
        {
            for( unsigned int i=0; i<pShaderFile->m_NumExposedUniforms; i++ )
            {
                ExposedUniformInfo* pInfo = &pShaderFile->m_ExposedUniforms[i];
                switch( pInfo->m_Type )
                {
                case ExposedUniformType_Float:
                    cJSON_AddNumberToObject( jMaterial, pInfo->m_Name, m_UniformValues[i].m_Float );
                    break;

                case ExposedUniformType_Vec2:
                    cJSONExt_AddFloatArrayToObject( jMaterial, pInfo->m_Name, m_UniformValues[i].m_Vec2, 2 );
                    break;

                case ExposedUniformType_Vec3:
                    cJSONExt_AddFloatArrayToObject( jMaterial, pInfo->m_Name, m_UniformValues[i].m_Vec3, 3 );
                    break;

                case ExposedUniformType_Vec4:
                    cJSONExt_AddFloatArrayToObject( jMaterial, pInfo->m_Name, m_UniformValues[i].m_Vec4, 4 );
                    break;

                case ExposedUniformType_ColorByte:
                    cJSONExt_AddUnsignedCharArrayToObject( jMaterial, pInfo->m_Name, m_UniformValues[i].m_ColorByte, 4 );
                    break;

                case ExposedUniformType_Sampler2D:
                    if( m_UniformValues[i].m_pTexture )
                    {
                        cJSON_AddStringToObject( jMaterial, pInfo->m_Name, m_UniformValues[i].m_pTexture->GetFilename() );
                    }
                    break;

                case ExposedUniformType_NotSet:
                default:
                    MyAssert( false );
                    break;
                }
            }
        }
    }
}

void MaterialDefinition::SetShaderInstanced(ShaderGroup* pShader)
{
    m_UnsavedChanges = true;

    if( pShader )
        pShader->AddRef();
    SAFE_RELEASE( m_pShaderGroupInstanced );
    m_pShaderGroupInstanced = pShader;
}

void MaterialDefinition::SetTextureColor(TextureDefinition* pTexture)
{
    m_UnsavedChanges = true;

    if( pTexture )
        pTexture->AddRef();
    SAFE_RELEASE( m_pTextureColor );
    m_pTextureColor = pTexture;
}

void MaterialDefinition::SetBlendType(MyRE::MaterialBlendTypes blendType)
{
    m_UnsavedChanges = true;
    m_BlendType = blendType;
}

void MaterialDefinition::SetColorAmbient(ColorByte color)
{
    m_UnsavedChanges = true;
    m_ColorAmbient = color;
}

void MaterialDefinition::SetColorDiffuse(ColorByte color)
{
    m_UnsavedChanges = true;
    m_ColorDiffuse = color;
}

void MaterialDefinition::SetColorSpecular(ColorByte color)
{
    m_UnsavedChanges = true;
    m_ColorSpecular = color;
}

void MaterialDefinition::SetUVScale(Vector2 scale)
{
    m_UnsavedChanges = true;
    m_UVScale = scale;
}

void MaterialDefinition::SetUVOffset(Vector2 offset)
{
    m_UnsavedChanges = true;
    m_UVOffset = offset;
}

bool MaterialDefinition::IsTransparent(BaseShader* pShader)
{
    if( m_BlendType == MyRE::MaterialBlendType_On )
        return true;

    // check the shader
    if( m_BlendType == MyRE::MaterialBlendType_UseShaderValue )
    {
        return pShader->m_BlendType == MyRE::MaterialBlendType_On ? true : false;
    }

    return false;
}

bool MaterialDefinition::IsTransparent()
{
    // Find first initialized shader from this group and return it's opacity setting.
    if( m_pShaderGroup )
    {
        for( int p=0; p<ShaderPass_NumTypes; p++ )
        {
            for( unsigned int lc=0; lc<ShaderGroup::SHADERGROUP_MAX_LIGHTS+1; lc++ )
            {
                for( unsigned int bc=0; bc<ShaderGroup::SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
                {
                    BaseShader* pShader = m_pShaderGroup->GetShader( (ShaderPassTypes)p, lc, bc );

                    if( pShader )
                    {
                        MyAssert( pShader->m_BlendType != MyRE::MaterialBlendType_UseShaderValue );

                        if( pShader->m_BlendType != MyRE::MaterialBlendType_NotSet )
                        {
                            return IsTransparent( pShader );
                        }
                    }
                }
            }
        }
    }

    // If no shader is initialized, consider it transparent.
    return true;
}

bool MaterialDefinition::IsEmissive(BaseShader* pShader)
{
    // check the shader
    return pShader->m_Emissive;
}

bool MaterialDefinition::IsEmissive()
{
    // Find first initialized shader from this group and return it's emissive setting.
    if( m_pShaderGroup )
    {
        for( int p=0; p<ShaderPass_NumTypes; p++ )
        {
            for( unsigned int lc=0; lc<ShaderGroup::SHADERGROUP_MAX_LIGHTS+1; lc++ )
            {
                for( unsigned int bc=0; bc<ShaderGroup::SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
                {
                    BaseShader* pShader = m_pShaderGroup->GetShader( (ShaderPassTypes)p, lc, bc );

                    if( pShader )
                    {
                        MyAssert( pShader->m_BlendType != MyRE::MaterialBlendType_UseShaderValue );

                        // If the shader is loaded, m_BlendType will be set.  TODO: Don't rely on blendtype.
                        if( pShader->m_BlendType != MyRE::MaterialBlendType_NotSet )
                        {
                            return IsEmissive( pShader );
                        }
                    }
                }
            }
        }
    }

    // If no shader is initialized, consider it not emissive.
    return false;
}

MyRE::MaterialBlendFactors MaterialDefinition::GetShaderBlendFactorSrc(BaseShader* pShader)
{
    if( pShader )
        return pShader->m_BlendFactorSrc;

    return MyRE::MaterialBlendFactor_SrcAlpha;
}

MyRE::MaterialBlendFactors MaterialDefinition::GetShaderBlendFactorSrc()
{
    if( m_pShaderGroup )
    {
        BaseShader* pShader = m_pShaderGroup->GetShader( ShaderPass_Main );
        if( pShader )
            return pShader->m_BlendFactorSrc;
    }

    return MyRE::MaterialBlendFactor_SrcAlpha;
}

GLenum MaterialDefinition::GetShaderBlendFactorSrc_OpenGL(BaseShader* pShader)
{
    MyRE::MaterialBlendFactors factor = GetShaderBlendFactorSrc( pShader );
    MyAssert( factor < MyRE::MaterialBlendFactor_NumTypes );
    return MaterialBlendFactors_OpenGL[factor];
}

GLenum MaterialDefinition::GetShaderBlendFactorSrc_OpenGL()
{
    MyRE::MaterialBlendFactors factor = GetShaderBlendFactorSrc();
    MyAssert( factor < MyRE::MaterialBlendFactor_NumTypes );
    return MaterialBlendFactors_OpenGL[factor];
}

MyRE::MaterialBlendFactors MaterialDefinition::GetShaderBlendFactorDest(BaseShader* pShader)
{
    if( pShader )
        return pShader->m_BlendFactorDest;

    return MyRE::MaterialBlendFactor_OneMinusSrcAlpha;
}

MyRE::MaterialBlendFactors MaterialDefinition::GetShaderBlendFactorDest()
{
    if( m_pShaderGroup )
    {
        BaseShader* pShader = m_pShaderGroup->GetShader( ShaderPass_Main );
        if( pShader )
            return pShader->m_BlendFactorDest;
    }

    return MyRE::MaterialBlendFactor_OneMinusSrcAlpha;
}

GLenum MaterialDefinition::GetShaderBlendFactorDest_OpenGL(BaseShader* pShader)
{
    MyRE::MaterialBlendFactors factor = GetShaderBlendFactorDest( pShader );
    MyAssert( factor < MyRE::MaterialBlendFactor_NumTypes );
    return MaterialBlendFactors_OpenGL[factor];
}

GLenum MaterialDefinition::GetShaderBlendFactorDest_OpenGL()
{
    MyRE::MaterialBlendFactors factor = GetShaderBlendFactorDest();
    MyAssert( factor < MyRE::MaterialBlendFactor_NumTypes );
    return MaterialBlendFactors_OpenGL[factor];
}

#if MYFW_EDITOR
void MaterialDefinition::OnPopupClick(MaterialDefinition* pMaterial, int id)
{
    MyFileObject* pMaterialFile = pMaterial->GetFile();

    switch( id )
    {
    case RightClick_ViewInWatchWindow:
        {
        }
        break;

    case RightClick_UnloadFile:
        {
            if( pMaterialFile )
                g_pFileManager->Editor_UnloadFile( pMaterialFile );
        }
        break;

    case RightClick_FindAllReferences:
        {
            if( pMaterialFile )
                g_pFileManager->Editor_FindAllReferences( pMaterialFile );
        }
        break;
    }
}

bool MaterialDefinition::IsReferencingFile(MyFileObject* pFile)
{
    if( GetTextureColor() && GetTextureColor()->GetFile() == pFile )
    {
        return true;
    }

    if( m_pShaderGroup )
    {
        MyFileObjectShader* pShaderFile = m_pShaderGroup->GetFile();

        if( pShaderFile == pFile )
            return true;

        if( pShaderFile )
        {
            for( unsigned int i=0; i<pShaderFile->m_NumExposedUniforms; i++ )
            {
                if( pShaderFile->m_ExposedUniforms[i].m_Type == ExposedUniformType_Sampler2D )
                {
                    if( m_UniformValues[i].m_pTexture && m_UniformValues[i].m_pTexture->GetFile() == pFile )
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void MaterialDefinition::SaveMaterial(const char* relativepath)
{
    if( m_Name[0] == '\0' )
        return;

    char filename[MAX_PATH];

    if( m_pFile != nullptr )
    {
        // If a file exists, use the existing file's fullpath.
        strcpy_s( filename, MAX_PATH, m_pFile->GetFullPath() );
    }
    else
    {
        // If a file doesn't exist, create the filename out of parts.
        // TODO: Move most of this block into generic system code.
        MyAssert( relativepath != nullptr );

        char workingdir[MAX_PATH];
#if MYFW_WINDOWS
        _getcwd( workingdir, MAX_PATH * sizeof(char) );
#else
        getcwd( workingdir, MAX_PATH * sizeof(char) );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/", workingdir, relativepath );
#if MYFW_WINDOWS
        CreateDirectoryA( filename, nullptr );
#else
        MyAssert( false );
#endif
        sprintf_s( filename, MAX_PATH, "%s/%s/%s.mymaterial", workingdir, relativepath, m_Name );

        // This is a new file, check for filename conflict.
        {
            unsigned int count = 0;
            char newname[MAX_MATERIAL_NAME_LEN];
            strcpy_s( newname, MAX_MATERIAL_NAME_LEN, m_Name );
            while( g_pFileManager->DoesFileExist( filename ) == true )
            {
                count++;

                sprintf_s( newname, MAX_MATERIAL_NAME_LEN, "%s(%d)", m_Name, count );
                sprintf_s( filename, MAX_PATH, "%s/%s/%s.mymaterial", workingdir, relativepath, newname );
            }
            strcpy_s( m_Name, MAX_MATERIAL_NAME_LEN, newname );
        }
    }

    // Create the json string to save into the material file.
    char* jsonstr = nullptr;
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
            cJSON_AddStringToObject( jMaterial, "TexColor", m_pTextureColor->GetFilename() );

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

        cJSON_AddNumberToObject( jMaterial, "PreviewType", m_PreviewType );        

        ExportExposedUniformValues( jMaterial );

        // dump material json structure to disk
        jsonstr = cJSON_Print( jRoot );
        cJSON_Delete( jRoot );
    }

    if( jsonstr != nullptr )
    {
        FILE* pFile = nullptr;
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

        // If the file managed to save, request it.
        if( m_pFile == nullptr )
        {
            sprintf_s( filename, MAX_PATH, "%s/%s.mymaterial", relativepath, m_Name );
            m_pFile = g_pFileManager->RequestFile( filename );
        }

        // Update timestamp when saving a file to disk, so it doesn't reload when alt-tabbing.
        if( m_pFile != nullptr )
        {
            m_pFile->UpdateTimestamp();
        }
    }

    m_UnsavedChanges = false;
}
#endif //MYFW_EDITOR
