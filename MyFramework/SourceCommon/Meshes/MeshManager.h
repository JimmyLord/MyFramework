//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MeshManager_H__
#define __MeshManager_H__

class GameCore;
class MeshManager;
class MaterialManager;
class MyFileObject;
class MyMesh;
class VertexFormatManager;

extern MeshManager* g_pMeshManager;

class MeshManager
{
protected:
    GameCore* m_pGameCore;
    TCPPListHead<MyMesh*> m_MeshList;

public:
    MeshManager(GameCore* pGameCore);

    // Getters.
    GameCore* GetGameCore() { return m_pGameCore; }
    VertexFormatManager* GetVertexFormatManager();
    MaterialManager* GetMaterialManager();

    // Add Mesh.
    void AddMesh(MyMesh* pMesh);

    // Find Mesh.
    MyMesh* FindMeshBySourceFile(MyFileObject* pFile);

    // Actions.
    void GuessAndAssignAppropriateShaderToMesh(MyMesh* pMesh);
};

#endif //__MeshManager_H__
