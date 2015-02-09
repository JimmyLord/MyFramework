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
    assert( name != 0 );

    Initialize();

    SetFileForAllPasses( pFile );
    g_pShaderGroupManager->AddShaderGroup( this );

#if MYFW_USING_WX
    g_pPanelMemory->AddShaderGroup( this, "ShaderGroups", name, StaticOnDrag );
#endif
}

void ShaderGroup::Initialize()
{
    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<MAX_LIGHTS+1; lc++ )
        {
            m_pShaderPasses[p][lc] = MyNew Shader_Base( (ShaderPassTypes)p );
        }
    }
}

ShaderGroup::~ShaderGroup()
{
    this->Remove(); // remove this node from the shadergroupmanager's list

    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<MAX_LIGHTS+1; lc++ )
        {
            SAFE_DELETE( m_pShaderPasses[p][lc] );
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

BaseShader* ShaderGroup::GlobalPass(int numlights)
{
    //return m_pShaderPasses[ShaderPass_Main];
    return GetShader( g_ActiveShaderPass, numlights );
}

BaseShader* ShaderGroup::GetShader(ShaderPassTypes pass, int numlights)
{
    if( numlights > MAX_LIGHTS )
    {
        LOGError( LOGTag, "ShaderGroup::GetShader() asking for too many lights\n" );
        numlights = MAX_LIGHTS;
    }

    // find the first shader that supports the correct number of lights or less.
    for( unsigned int lc = numlights; lc >= 0; lc-- )
    {
        if( m_pShaderPasses[pass][lc] != 0 )
            return m_pShaderPasses[pass][lc];
    }

    return 0;
}

void ShaderGroup::SetFileForAllPasses(MyFileObject* pFile)
{
    for( int p=0; p<ShaderPass_NumTypes; p++ )
    {
        for( unsigned int lc=0; lc<MAX_LIGHTS+1; lc++ )
        {
            if( m_pShaderPasses[p][lc] )
            {
                m_pShaderPasses[p][lc]->m_pFile = pFile;
                pFile->AddRef();

                if( lc == 0 )
                    m_pShaderPasses[p][lc]->OverridePredefs( "#define NUM_LIGHTS 0\n", "#define NUM_LIGHTS 0\n", true );
                if( lc == 1 )
                    m_pShaderPasses[p][lc]->OverridePredefs( "#define NUM_LIGHTS 1\n", "#define NUM_LIGHTS 1\n", true );
                if( lc == 2 )
                    m_pShaderPasses[p][lc]->OverridePredefs( "#define NUM_LIGHTS 2\n", "#define NUM_LIGHTS 2\n", true );
                if( lc == 3 )
                    m_pShaderPasses[p][lc]->OverridePredefs( "#define NUM_LIGHTS 3\n", "#define NUM_LIGHTS 3\n", true );
                if( lc == 4)
                    m_pShaderPasses[p][lc]->OverridePredefs( "#define NUM_LIGHTS 4\n", "#define NUM_LIGHTS 4\n", true );
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
