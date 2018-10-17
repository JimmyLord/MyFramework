//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SceneGraph_Octree_H__
#define __SceneGraph_Octree_H__

class SceneGraph_Octree;

class OctreeNode
{
    friend class SceneGraph_Octree;

protected:
    SceneGraph_Octree* m_pSceneGraph;
    unsigned int m_NodeDepth;

    MyAABounds m_Bounds;

    OctreeNode* m_pChildNodes[8];
    OctreeNode* m_pParentNode;

    CPPListHead m_Renderables;

    void Cleanup();

public:
    OctreeNode();
    ~OctreeNode();
};

class SceneGraph_Octree : public SceneGraph_Base
{
    friend class OctreeNode;

protected:
    unsigned int m_MaxDepth;

    MySimplePool<OctreeNode> m_OctreeNodePool; 

    OctreeNode* m_pRootNode;

    bool m_Dirty;

    void UpdateTree(OctreeNode* pOctreeNode);
    void CollapseChildNodes(OctreeNode* pOctreeNode);
    void DrawNode(OctreeNode* pOctreeNode, bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr pPreDrawCallbackFunc);

public:
    SceneGraph_Octree(unsigned int treedepth, float minx, float miny, float minz, float maxx, float maxy, float maxz);
    virtual ~SceneGraph_Octree();

    void Resize(float minx, float miny, float minz, float maxx, float maxy, float maxz);

    virtual SceneGraphObject* AddObjectWithFlagOverride(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitiveType, int pointSize, SceneGraphFlags flags, unsigned int layers, void* pUserData);
    virtual void RemoveObject(SceneGraphObject* pObject);

    virtual void ObjectMoved(SceneGraphObject* pObject);

    virtual void Draw(bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr pPreDrawCallbackFunc);
};

#endif //__SceneGraph_Octree_H__