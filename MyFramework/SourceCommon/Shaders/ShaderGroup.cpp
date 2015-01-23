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

ShaderGroup::ShaderGroup(char* name)
: m_Name(name)
{
    SetShaders( 0, 0, 0 );
    g_pShaderGroupManager->AddShaderGroup( this );

#if MYFW_USING_WX
    g_pPanelMemory->AddShaderGroup( this, "ShaderGroups", name, StaticOnDrag );
#endif
}

ShaderGroup::ShaderGroup(BaseShader* pMainPass, char* name)
: m_Name(name)
{
    SetShaders( pMainPass, 0, 0 );
    g_pShaderGroupManager->AddShaderGroup( this );

#if MYFW_USING_WX
    g_pPanelMemory->AddShaderGroup( this, "ShaderGroups", name, StaticOnDrag );
#endif
}

ShaderGroup::ShaderGroup(BaseShader* pMainPass, BaseShader* pMainPassNoShadow, BaseShader* pShadowCastRGBAPass, char* name)
: m_Name(name)
{
    SetShaders( pMainPass, pMainPassNoShadow, pShadowCastRGBAPass );
    g_pShaderGroupManager->AddShaderGroup( this );

#if MYFW_USING_WX
    g_pPanelMemory->AddShaderGroup( this, "ShaderGroups", name, StaticOnDrag );
#endif
}

ShaderGroup::~ShaderGroup()
{
    this->Remove(); // remove this node from the shadergroupmanager's list

    for( int i=0; i<ShaderPass_NumTypes; i++ )
        SAFE_DELETE( m_pShaderPasses[i] );    

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

BaseShader* ShaderGroup::GlobalPass()
{
    //return m_pShaderPasses[ShaderPass_Main];
    return m_pShaderPasses[g_ActiveShaderPass];
}

void ShaderGroup::SetShader(ShaderPassTypes pass, BaseShader* pShader)
{
    m_pShaderPasses[pass] = pShader;
}

void ShaderGroup::SetShaders(BaseShader* pMainPass, BaseShader* pMainPassNoShadow, BaseShader* pShadowCastRGBAPass)
{
    m_pShaderPasses[ShaderPass_Main] = pMainPass;
    m_pShaderPasses[ShaderPass_MainNoReceiveShadows] = pMainPassNoShadow;
    m_pShaderPasses[ShaderPass_ShadowCastRGBA] = pShadowCastRGBAPass;
}

void ShaderGroup::SetFileForAllPasses(const char* pFilename)
{
    for( int i=0; i<ShaderPass_NumTypes; i++ )
    {
        if( m_pShaderPasses[i] )
            m_pShaderPasses[i]->m_pFilename = pFilename;
    }
}

void ShaderGroup::SetFileForAllPasses(MyFileObject* pFile)
{
    for( int i=0; i<ShaderPass_NumTypes; i++ )
    {
        if( m_pShaderPasses[i] )
        {
            m_pShaderPasses[i]->m_pFile = pFile;
            pFile->AddRef();
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
