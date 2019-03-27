//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MeshManager.h"
#include "MyMesh.h"
#include "MySubmesh.h"
#include "../Core/GameCore.h"
#include "../Textures/MaterialDefinition.h"

MeshManager::MeshManager(GameCore* pGameCore)
{
    m_pGameCore = pGameCore;
}

VertexFormatManager* MeshManager::GetVertexFormatManager()
{
    return m_pGameCore->GetManagers()->GetVertexFormatManager();
}

MaterialManager* MeshManager::GetMaterialManager()
{
    return m_pGameCore->GetManagers()->GetMaterialManager();
}

void MeshManager::AddMesh(MyMesh* pMesh)
{
    m_MeshList.AddTail( pMesh );
}

MyMesh* MeshManager::FindMeshBySourceFile(MyFileObject* pFile)
{
    for( MyMesh* pMesh = m_MeshList.GetHead(); pMesh; pMesh = pMesh->GetNext() )
    {
        if( pMesh->GetFile() == pFile )
            return pMesh;
    }

    return nullptr;
}

void MeshManager::GuessAndAssignAppropriateShaderToMesh(MyMesh* pMesh)
{
    ShaderGroupManager* pShaderGroupManager = m_pGameCore->GetManagers()->GetShaderGroupManager();

    for( unsigned int i=0; i<pMesh->m_SubmeshList.Count(); i++ )
    {
        if( pMesh->m_SubmeshList[i]->GetMaterial()->GetShader() == nullptr )
        {
            // TODO: Actually write code here...
            pMesh->m_SubmeshList[i]->GetMaterial()->SetShader( pShaderGroupManager->FindShaderGroupByName( "Shader_TintColor" ) );
        }
    }
}
