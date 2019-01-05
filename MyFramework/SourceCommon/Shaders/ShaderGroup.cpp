//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "../Renderers/BaseClasses/Shader_Base.h"

// TODO: Fix GL Includes.
#include "../Renderers/OpenGL/Shader_OpenGL.h"

ShaderGroupManager* g_pShaderGroupManager = nullptr;
ShaderPassTypes g_ActiveShaderPass = ShaderPass_Main;

const char* g_ShaderPassDefines[ShaderPass_NumTypes] =
{
    "#define PassMain 1\n#define ReceiveShadows 1\n",
    "#define PassMain 1\n#define ReceiveShadows 0\n#define Deferred 1\n",
    "#define PassMain 1\n#define ReceiveShadows 0\n",
    "#define PassShadowCastRGB 1\n",
};

ShaderGroup::ShaderGroup()
{
    Create( 0, 0 );
}

ShaderGroup::ShaderGroup(const char* pFilename)
{
    MyFileObject* pShaderFile = RequestFile( pFilename );
    Create( pShaderFile, 0 );
    pShaderFile->Release();
}

ShaderGroup::ShaderGroup(MyFileObject* pFile)
{
    Create( pFile, 0 );
}

ShaderGroup::ShaderGroup(MyFileObject* pFile, ShaderGroupShaderAllocationFunction pFunc)
{
    Create( pFile, pFunc );
}

void ShaderGroup::Create(MyFileObject* pFile, ShaderGroupShaderAllocationFunction pFunc)
{
    if( pFile && pFile->IsA( "MyFileShader" ) == false )
    {
        MyAssert( false );
        return;
    }

    MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pFile;

    m_pFile = pShaderFile;
    if( m_pFile )
        m_pFile->AddRef();

    m_pShaderAllocationFunction = pFunc;

    Initialize();

    SetFileForAllPasses( pFile );
    g_pShaderGroupManager->AddShaderGroup( this );
}

void ShaderGroup::Initialize()
{
    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
        {
            for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
            {
                if( m_pShaderAllocationFunction )
                    m_pShaderPasses[p][lc][bc] = m_pShaderAllocationFunction( (ShaderPassTypes)p );
                else
                    m_pShaderPasses[p][lc][bc] = MyNew Shader_OpenGL( (ShaderPassTypes)p );
            }
        }
    }
}

bool ShaderGroup::ContainsShader(BaseShader* pShader)
{
    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
        {
            for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
            {
                if( m_pShaderPasses[p][lc][bc] == pShader )
                    return true;
            }
        }
    }

    return false;
}

void ShaderGroup::OverridePassTypeForAllShaders(ShaderPassTypes originalPassType, ShaderPassTypes newPassType)
{
    // This must be called before the shader gets compiled since m_PassType is used to determine the
    //   g_ShaderPassDefines[] string used in BaseShader::LoadAndCompile(...).

    int p = originalPassType;

    for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
    {
        for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
        {
            MyAssert( m_pShaderPasses[p][lc][bc]->m_Initialized == false );

            m_pShaderPasses[p][lc][bc]->m_PassType = newPassType;
        }
    }
}

void ShaderGroup::DisableShadowCasting_AndDoItBadly_WillBeReplaced()
{
    int p = ShaderPass_ShadowCastRGBA;

    for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
    {
        for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
        {
            SAFE_DELETE( m_pShaderPasses[p][lc][bc] );
        }
    }
}

ShaderGroup::~ShaderGroup()
{
    this->Remove(); // Remove this node from the ShaderGroupManager's list.

    SAFE_RELEASE( m_pFile );

    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
        {
            for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
            {
                SAFE_DELETE( m_pShaderPasses[p][lc][bc] );
            }
        }
    }
}

#if MYFW_EDITOR
void ShaderGroup::OnPopupClick(ShaderGroup* pShaderGroup, int id)
{
    MyFileObject* pFileObject = pShaderGroup->GetFile();

    switch( id )
    {
    case RightClick_OpenFile:
        {
            pFileObject->OSLaunchFile( true );
        }
        break;

    case RightClick_UnloadFile:
        {
            if( pFileObject )
                g_pFileManager->Editor_UnloadFile( pFileObject );
        }
        break;

    case RightClick_FindAllReferences:
        {
            if( pFileObject )
                g_pFileManager->Editor_FindAllReferences( pFileObject );
        }
        break;
    }
}
#endif //MYFW_EDITOR

BaseShader* ShaderGroup::GlobalPass(int numLights, int numBones)
{
    return GetShader( g_ActiveShaderPass, numLights, numBones );
}

BaseShader* ShaderGroup::GetShader(ShaderPassTypes pass, int numLights, int numBones)
{
    if( numLights > SHADERGROUP_MAX_LIGHTS )
    {
        LOGError( LOGTag, "ShaderGroup::GetShader() asking for too many lights\n" );
        numLights = SHADERGROUP_MAX_LIGHTS;
    }

    if( numBones > SHADERGROUP_MAX_BONE_INFLUENCES )
    {
        LOGError( LOGTag, "ShaderGroup::GetShader() asking for too many bones\n" );
        numBones = SHADERGROUP_MAX_BONE_INFLUENCES;
    }

    // Find the first shader that supports the correct number of lights/bones or less.
    for( int lc = numLights; lc >= 0; lc-- )
    {
        for( int bc = numBones; bc >= 0; bc-- )
        {
            if( m_pShaderPasses[pass][lc][bc] != nullptr )
                return m_pShaderPasses[pass][lc][bc];
        }
    }

    return 0;
}

void ShaderGroup::SetFileForAllPasses(MyFileObject* pFile)
{
    if( pFile == nullptr )
        return;

    if( pFile->IsA( "MyFileShader" ) == false )
    {
        MyAssert( false );
        return;
    }

    MyFileObjectShader* pFileShader = (MyFileObjectShader*)pFile;

    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
        {
            for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
            {
                char tempstr[60]; // Big enough to hold "#define NUM_LIGHTS 0\n#define NUM_INF_BONES 0\n".

                if( m_pShaderPasses[p][lc] )
                {
                    m_pShaderPasses[p][lc][bc]->m_pFile = pFileShader;
                    pFileShader->AddRef();

                    sprintf_s( tempstr, 50, "#define NUM_LIGHTS %d\n#define NUM_INF_BONES %d\n", lc, bc );

                    m_pShaderPasses[p][lc][bc]->OverridePredefs( tempstr, tempstr, tempstr, true );
                }
            }
        }
    }
}

void ShaderGroupManager::AddShaderGroup(ShaderGroup* pShaderGroup)
{
    m_ShaderGroupList.AddTail( pShaderGroup );
}

ShaderGroup* ShaderGroupManager::FindShaderGroupByName(const char* name)
{
    MyAssert( name );

    for( ShaderGroup* pShaderGroup = m_ShaderGroupList.GetHead(); pShaderGroup; pShaderGroup = pShaderGroup->GetNext() )
    {
        if( strcmp( pShaderGroup->GetName(), name ) == 0 )
        {
            return pShaderGroup;
        }
    }

    return 0;
}

ShaderGroup* ShaderGroupManager::FindShaderGroupByFile(MyFileObject* pFile)
{
    MyAssert( pFile );

    for( ShaderGroup* pShaderGroup = m_ShaderGroupList.GetHead(); pShaderGroup; pShaderGroup = pShaderGroup->GetNext() )
    {
        if( pShaderGroup->GetShader( ShaderPass_Main, 0, 0 )->m_pFile == pFile )
        {
            return pShaderGroup;
        }
    }

    return 0;
}

ShaderGroup* ShaderGroupManager::FindShaderGroupByFilename(const char* fullpath)
{
    MyAssert( fullpath );

    for( ShaderGroup* pShaderGroup = m_ShaderGroupList.GetHead(); pShaderGroup; pShaderGroup = pShaderGroup->GetNext() )
    {
        MyFileObject* pFile = pShaderGroup->GetFile();
        if( pFile && strcmp( pFile->GetFullPath(), fullpath ) == 0 )
        {
            return pShaderGroup;
        }
    }

    return 0;
}
