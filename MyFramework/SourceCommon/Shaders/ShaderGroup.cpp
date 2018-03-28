//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
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

ShaderGroup::ShaderGroup()
{
    Create( 0, 0 );
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

#if MYFW_USING_WX
    if( pShaderFile && pShaderFile->m_IsAnIncludeFile == false )
    {
        MyAssert( m_pFile->GetFilenameWithoutExtension() != 0 );

        if( pShaderFile->MemoryPanel_IsVisible() )
        {
            g_pPanelMemory->AddShaderGroup( this, "ShaderGroups", m_pFile->GetFilenameWithoutExtension(), StaticOnRightClick, StaticOnDrag );
        }
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
                if( m_pShaderAllocationFunction )
                    m_pShaderPasses[p][lc][bc] = m_pShaderAllocationFunction( (ShaderPassTypes)p );
                else
                    m_pShaderPasses[p][lc][bc] = MyNew Shader_Base( (ShaderPassTypes)p );
            }
        }
    }
}

void ShaderGroup::OverridePassTypeForAllShaders(ShaderPassTypes originalpasstype, ShaderPassTypes newpasstype)
{
    // This must be called before the shader gets compiled since m_PassType is used to determine the
    //   g_ShaderPassDefines[] string used in BaseShader::LoadAndCompile(...);

    int p = originalpasstype;

    for( unsigned int lc=0; lc<SHADERGROUP_MAX_LIGHTS+1; lc++ )
    {
        for( unsigned int bc=0; bc<SHADERGROUP_MAX_BONE_INFLUENCES+1; bc++ )
        {
            MyAssert( m_pShaderPasses[p][lc][bc]->m_Initialized == false );

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

#if MYFW_USING_WX
    if( g_pPanelMemory )
        g_pPanelMemory->RemoveShaderGroup( this );
#endif
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

#if MYFW_USING_WX
void ShaderGroup::OnLeftClick(unsigned int count) // StaticOnLeftClick
{
}

void ShaderGroup::OnRightClick() // StaticOnRightClick
{
 	wxMenu menu;
    menu.SetClientData( this );
    
    if( GetFile() )
    {
        menu.Append( RightClick_OpenFile, "Open File" );
        menu.Append( RightClick_UnloadFile, "Unload File" );
        menu.Append( RightClick_FindAllReferences, wxString::Format( wxT("Find References (%d)"), (long long)this->GetRefCount() ) );
    }

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ShaderGroup::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ShaderGroup::OnPopupClick(wxEvent &evt)
{
    ShaderGroup* pShaderGroup = (ShaderGroup*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    int id = evt.GetId();
    OnPopupClick( pShaderGroup, id );
}

void ShaderGroup::OnDrag() // StaticOnDrag
{
    g_DragAndDropStruct.Add( DragAndDropType_ShaderGroupPointer, this );
}
#endif //MYFW_USING_WX
#endif //MYFW_EDITOR

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
    if( pFile == 0 )
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
                char tempstr[60]; // big enough to hold "#define NUM_LIGHTS 0\n#define NUM_INF_BONES 0\n"

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
    MyAssert( pFile );

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

ShaderGroup* ShaderGroupManager::FindShaderGroupByFilename(const char* fullpath)
{
    MyAssert( fullpath );

    for( CPPListNode* pNode = m_ShaderGroupList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)pNode;

        MyFileObject* pFile = pShaderGroup->GetFile();
        if( pFile && strcmp( pFile->GetFullPath(), fullpath ) == 0 )
        {
            return pShaderGroup;
        }
    }

    return 0;
}
