//
// Copyright (c) 2017-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __RenderGraph_Octree_H__
#define __RenderGraph_Octree_H__

#include "RenderGraph_Base.h"
#include "../DataTypes/MyAABounds.h"
#include "../DataTypes/MyActivePool.h"
#include "../Renderers/BaseClasses/Renderer_Enums.h"

class RenderGraph_Octree;

class OctreeNode
{
    friend class RenderGraph_Octree;

protected:
    RenderGraph_Octree* m_pRenderGraph;
    unsigned int m_NodeDepth;

    MyAABounds m_Bounds;

    OctreeNode* m_pChildNodes[8];
    OctreeNode* m_pParentNode;

    TCPPListHead<RenderGraphObject*> m_Renderables;

    void Cleanup();

public:
    OctreeNode();
    ~OctreeNode();
};

class RenderGraph_Octree : public RenderGraph_Base
{
    friend class OctreeNode;

protected:
    unsigned int m_MaxDepth;

    MySimplePool<OctreeNode> m_OctreeNodePool; 

    OctreeNode* m_pRootNode;

    bool m_Dirty;

    void UpdateTree(OctreeNode* pOctreeNode);
    void CollapseChildNodes(OctreeNode* pOctreeNode);
    void DrawNode(OctreeNode* pOctreeNode, bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr* pPreDrawCallbackFunc);

public:
    RenderGraph_Octree(GameCore* pGameCore, uint32 treeDepth, float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
    virtual ~RenderGraph_Octree();

    void Resize(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

    virtual RenderGraphObject* AddObjectWithFlagOverride(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, MyRE::PrimitiveTypes primitiveType, int pointSize, RenderGraphFlags flags, unsigned int layers, void* pUserData);
    virtual void RemoveObject(RenderGraphObject* pObject);

    virtual void ObjectMoved(RenderGraphObject* pObject);

    virtual void Draw(bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr* pPreDrawCallbackFunc);
};

#endif //__RenderGraph_Octree_H__