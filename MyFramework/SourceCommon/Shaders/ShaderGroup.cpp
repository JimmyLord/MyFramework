//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

ShaderGroupManager* g_pShaderGroupManager = 0;
ShaderPassTypes g_ActiveShaderPass = ShaderPass_Main;

const char* g_ShaderPassDefines[ShaderPass_NumTypes] =
{
    "#define PassMain 1\n#define ReceiveShadows 1\n",
    "#define PassMain 1\n#define ReceiveShadows 0\n",
    "#define PassShadowCastRGB 1\n",
};

ShaderGroup::ShaderGroup(MyFileObject* pFile, char* name)
: m_Name(name)
{
    MyFileObjectShader* pShaderFile = dynamic_cast<MyFileObjectShader*>( pFile );
    assert( pShaderFile );

    Initialize();

    SetFileForAllPasses( pFile );
    g_pShaderGroupManager->AddShaderGroup( this );

#if MYFW_USING_WX
    if( pShaderFile->m_IsAnIncludeFile == false )
    {
        assert( name != 0 );
        g_pPanelMemory->AddShaderGroup( this, "ShaderGroups", name, StaticOnDrag );
    }
#endif
}

void ShaderGroup::Initialize()
{
    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
        {
            for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
            {
                m_pShaderPasses[p][lc][bc] = MyNew Shader_Base( (ShaderPassTypes)p );
            }
        }
    }
}

void ShaderGroup::OverridePassTypeForAllShaders(ShaderPassTypes originalpasstype, ShaderPassTypes newpasstype)
{
    int p = originalpasstype;

    for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
    {
        for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
        {
            m_pShaderPasses[p][lc][bc]->m_PassType = newpasstype;
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
    this->Remove(); // remove this node from the shadergroupmanager's list

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

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveShaderGroup( this );
#endif
}

#if MYFW_USING_WX
void ShaderGroup::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_ShaderGroupPointer;
    g_DragAndDropStruct.m_Value = this;
}
#endif //MYFW_USING_WX

BaseShader* ShaderGroup::GlobalPass(int numlights, int numbones)
{
    //return m_pShaderPasses[ShaderPass_Main];
    return GetShader( g_ActiveShaderPass, numlights, numbones );
}

BaseShader* ShaderGroup::GetShader(ShaderPassTypes pass, int numlights, int numbones)
{
    if( numlights > SHADERGROUP_MAX_LIGHTS )
    {
        LOGError( LOGTag, "ShaderGroup::GetShader() asking for too many lights\n" );
        numlights = SHADERGROUP_MAX_LIGHTS;
    }

    if( numbones > SHADERGROUP_MAX_BONE_INFLUENCES )
    {
        LOGError( LOGTag, "ShaderGroup::GetShader() asking for too many bones\n" );
        numbones = SHADERGROUP_MAX_BONE_INFLUENCES;
    }

    // find the first shader that supports the correct number of lights/bones or less.
    for( int lc = numlights; lc >= 0; lc-- )
    {
        for( int bc = numbones; bc >= 0; bc-- )
        {
            if( m_pShaderPasses[pass][lc][bc] != 0 )
                return m_pShaderPasses[pass][lc][bc];
        }
    }

    return 0;
}

void ShaderGroup::SetFileForAllPasses(MyFileObject* pFile)
{
    MyFileObjectShader* pFileShader = dynamic_cast<MyFileObjectShader*>( pFile );
    assert( pFileShader );
    if( pFileShader == 0 )
        return;

    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
        {
            for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
            {
                char tempstr[60]; // big enough to hold "#define NUM_LIGHTS 0\n#define NUM_INF_BONES 0\n"

                if( m_pShaderPasses[p][lc] )
                {
                    m_pShaderPasses[p][lc][bc]->m_pFile = pFileShader;
                    pFileShader->AddRef();

                    sprintf_s( tempstr, 50, "#define NUM_LIGHTS %d\n#define NUM_INF_BONES %d\n", lc, bc );

                    m_pShaderPasses[p][lc][bc]->OverridePredefs( tempstr, tempstr, true );
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
    assert( name );

    for( CPPListNode* pNode = m_ShaderGroupList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)pNode;

        if( strcmp( pShaderGroup->GetName(), name ) == 0 )
        {
            return pShaderGroup;
        }
    }

    return 0;
}

ShaderGroup* ShaderGroupManager::FindShaderGroupByFile(MyFileObject* pFile)
{
    assert( pFile );

    for( CPPListNode* pNode = m_ShaderGroupList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)pNode;

        if( pShaderGroup->GetShader( ShaderPass_Main, 0, 0 )->m_pFile == pFile )
        {
            return pShaderGroup;
        }
    }

    return 0;
}
