//
// Copyright (c) 2017-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "SceneGraph_Octree.h"
#include "../Core/GameCore.h"
#include "../DataTypes/MyAABounds.h"
#include "../DataTypes/MyActivePool.h"
#include "../Meshes/LightManager.h"
#include "../Meshes/MyMesh.h"
#include "../Meshes/MySubmesh.h"
#include "../Textures/MaterialManager.h"

OctreeNode::OctreeNode()
{
    m_pSceneGraph = nullptr;

    for( int i=0; i<8; i++ )
    {
        m_pChildNodes[i] = nullptr;
    }
    m_pParentNode = nullptr;
}

OctreeNode::~OctreeNode()
{
    MyAssert( m_pSceneGraph == nullptr );
    MyAssert( m_pParentNode == nullptr );    
}

void OctreeNode::Cleanup()
{
    while( m_Renderables.GetHead() )
    {
        m_pSceneGraph->m_pObjectPool.ReturnObjectToPool( (SceneGraphObject*)m_Renderables.RemHead() );
    }

    for( int i=0; i<8; i++ )
    {
        if( m_pChildNodes[i] )
        {
            m_pChildNodes[i]->Cleanup();
            m_pChildNodes[i] = nullptr;
        }
    }

    m_pSceneGraph->m_OctreeNodePool.ReturnObjectToPool( this );
    m_pSceneGraph = nullptr;
    m_pParentNode = nullptr;
}

SceneGraph_Octree::SceneGraph_Octree(GameCore* pGameCore, uint32 treeDepth, float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
: SceneGraph_Base( pGameCore )
{
    m_MaxDepth = treeDepth;

    int maxNodes = 0;
    int numAtDepth = 1;
    for( unsigned int i=0; i<treeDepth; i++ )
    {
        maxNodes += numAtDepth;
        numAtDepth *= 8;
    }

    m_Dirty = false;

    m_OctreeNodePool.AllocateObjects( maxNodes );

    m_pRootNode = m_OctreeNodePool.GetObjectFromPool();

    Vector3 halfSize( (maxX - minX)/2.0f, (maxY - minY)/2.0f, (maxZ - minZ)/2.0f );
    Vector3 center( minX + halfSize.x, minY + halfSize.y, minZ + halfSize.z );

    m_pRootNode->m_Bounds.Set( center, halfSize );
    m_pRootNode->m_pSceneGraph = this;
    m_pRootNode->m_NodeDepth = 0;
}

SceneGraph_Octree::~SceneGraph_Octree()
{
    m_pRootNode->Cleanup();
}

bool FitsInsideAABB(MyAABounds* pOuterBounds, MyAABounds* pInnerBounds, Vector3 innerOffset)
{
    Vector3 outerCenter = pOuterBounds->GetCenter();
    Vector3 outerHalfSize = pOuterBounds->GetHalfSize();
    Vector3 innerCenter = pInnerBounds->GetCenter();
    Vector3 innerHalfSize = pInnerBounds->GetHalfSize();

    Vector3 outerMin = outerCenter - outerHalfSize;
    Vector3 outerMax = outerCenter + outerHalfSize;
    Vector3 innerMin = innerCenter - innerHalfSize + innerOffset;
    Vector3 innerMax = innerCenter + innerHalfSize + innerOffset;
    
    if( innerMin.x < outerMin.x ) return false;
    if( innerMin.y < outerMin.y ) return false;
    if( innerMin.z < outerMin.z ) return false;
    if( innerMax.x > outerMax.x ) return false;
    if( innerMax.y > outerMax.y ) return false;
    if( innerMax.z > outerMax.z ) return false;

    return true;
}

bool FitsInFrustum(MyAABounds* pBounds, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld)
{
    Vector3 center = pBounds->GetCenter();
    Vector3 half = pBounds->GetHalfSize();

    // Create a wvp matrix for pBounds.
    MyMatrix wvp = *pMatProj * *pMatView;
    if( pMatWorld )
        wvp = wvp * *pMatWorld;

    Vector4 clipPos[8];

    // Transform AABB extents into clip space.
    clipPos[0] = wvp * Vector4(center.x - half.x, center.y - half.y, center.z - half.z, 1);
    clipPos[1] = wvp * Vector4(center.x - half.x, center.y - half.y, center.z + half.z, 1);
    clipPos[2] = wvp * Vector4(center.x - half.x, center.y + half.y, center.z - half.z, 1);
    clipPos[3] = wvp * Vector4(center.x - half.x, center.y + half.y, center.z + half.z, 1);
    clipPos[4] = wvp * Vector4(center.x + half.x, center.y - half.y, center.z - half.z, 1);
    clipPos[5] = wvp * Vector4(center.x + half.x, center.y - half.y, center.z + half.z, 1);
    clipPos[6] = wvp * Vector4(center.x + half.x, center.y + half.y, center.z - half.z, 1);
    clipPos[7] = wvp * Vector4(center.x + half.x, center.y + half.y, center.z + half.z, 1);

    // Check visibility two planes at a time.
    bool visible = true;
    for( int component=0; component<3; component++ ) // Loop through x/y/z.
    {
        // Check if all 8 points are less than the -w extent of it's axis.
        visible = false;
        for( int i=0; i<8; i++ )
        {
            if( clipPos[i][component] >= -clipPos[i].w )
            {
                visible = true; // This point is on the visible side of the plane, skip to next plane.
                break;
            }
        }
        if( visible == false ) // All points are on outside of plane, don't draw object.
            break;

        // Check if all 8 points are greater than the w extent of it's axis.
        visible = false;
        for( int i=0; i<8; i++ )
        {
            if( clipPos[i][component] <= clipPos[i].w )
            {
                visible = true; // This point is on the visible side of the plane, skip to next plane.
                break;
            }
        }
        if( visible == false ) // All points are on outside of plane, don't draw object.
            break;
    }

    // If all points are on outside of frustum, don't draw mesh.
    if( visible == false )
        return false;

    return true;
}

void SceneGraph_Octree::UpdateTree(OctreeNode* pOctreeNode)
{
    if( pOctreeNode->m_NodeDepth >= m_MaxDepth - 1 )
        return;

    // Move all objects in pOctreeNode node down as far as they can go.
    SceneGraphObject* pNextSceneGraphObject;
    for( SceneGraphObject* pObject = pOctreeNode->m_Renderables.GetHead(); pObject; pObject = pNextSceneGraphObject )
    {
        pNextSceneGraphObject = pObject->GetNext();

        if( pObject->m_pMesh == nullptr )
            continue;

        MyAABounds* meshBounds = pObject->m_pMesh->GetBounds();
        Vector3 meshPos = pObject->m_pTransform->GetTranslation();

        for( int i=0; i<8; i++ )
        {
            MyAABounds childBounds;

            if( pOctreeNode->m_pChildNodes[i] == nullptr )
            {
                MyAABounds* currentBounds = &pOctreeNode->m_Bounds;
                Vector3 center = currentBounds->GetCenter();
                Vector3 quarterSize = currentBounds->GetHalfSize()/2;

                if( i == 0 ) childBounds.Set( Vector3( center.x - quarterSize.x, center.y + quarterSize.y, center.z + quarterSize.z ), quarterSize );
                if( i == 1 ) childBounds.Set( Vector3( center.x + quarterSize.x, center.y + quarterSize.y, center.z + quarterSize.z ), quarterSize );
                if( i == 2 ) childBounds.Set( Vector3( center.x - quarterSize.x, center.y + quarterSize.y, center.z - quarterSize.z ), quarterSize );
                if( i == 3 ) childBounds.Set( Vector3( center.x + quarterSize.x, center.y + quarterSize.y, center.z - quarterSize.z ), quarterSize );
                if( i == 4 ) childBounds.Set( Vector3( center.x - quarterSize.x, center.y - quarterSize.y, center.z + quarterSize.z ), quarterSize );
                if( i == 5 ) childBounds.Set( Vector3( center.x + quarterSize.x, center.y - quarterSize.y, center.z + quarterSize.z ), quarterSize );
                if( i == 6 ) childBounds.Set( Vector3( center.x - quarterSize.x, center.y - quarterSize.y, center.z - quarterSize.z ), quarterSize );
                if( i == 7 ) childBounds.Set( Vector3( center.x + quarterSize.x, center.y - quarterSize.y, center.z - quarterSize.z ), quarterSize );
            }
            else
            {
                childBounds = pOctreeNode->m_pChildNodes[i]->m_Bounds;
            }

            if( FitsInsideAABB( &childBounds, meshBounds, meshPos ) )
            {
                if( pOctreeNode->m_pChildNodes[i] == nullptr )
                {
                    pOctreeNode->m_pChildNodes[i] = m_OctreeNodePool.GetObjectFromPool();
                    pOctreeNode->m_pChildNodes[i]->m_Bounds = childBounds;
                    pOctreeNode->m_pChildNodes[i]->m_pParentNode = pOctreeNode;
                    pOctreeNode->m_pChildNodes[i]->m_pSceneGraph = this;
                    pOctreeNode->m_pChildNodes[i]->m_NodeDepth = pOctreeNode->m_NodeDepth + 1;
                }

                pOctreeNode->m_pChildNodes[i]->m_Renderables.MoveTail( pObject );
            }
        }
    }

    // Recurse through children.
    for( int i=0; i<8; i++ )
    {
        if( pOctreeNode->m_pChildNodes[i] != nullptr )
        {
            UpdateTree( pOctreeNode->m_pChildNodes[i] );
        }
    }
}

void SceneGraph_Octree::CollapseChildNodes(OctreeNode* pOctreeNode)
{
    // Loop through the 8 child nodes.
    for( int i=0; i<8; i++ )
    {
        if( pOctreeNode->m_pChildNodes[i] != nullptr )
        {
            // Move all SceneGraphObjects from each child node onto the tail of the root node.
            TCPPListHead<SceneGraphObject*>* childObjectList = &pOctreeNode->m_pChildNodes[i]->m_Renderables;
            if( childObjectList->GetHead() )
            {
                m_pRootNode->m_Renderables.BulkMoveTail( childObjectList->GetHead(), childObjectList->GetTail() );
            }

            // Recurse through children.
            CollapseChildNodes( pOctreeNode->m_pChildNodes[i] );

            // Return this child to the node pool.
            m_OctreeNodePool.ReturnObjectToPool( pOctreeNode->m_pChildNodes[i] );
            pOctreeNode->m_pChildNodes[i]->m_pSceneGraph = nullptr;
            pOctreeNode->m_pChildNodes[i]->m_pParentNode = nullptr;
            pOctreeNode->m_pChildNodes[i] = nullptr;
        }
    }
}

void SceneGraph_Octree::Resize(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
    MyAABounds newBounds;
    Vector3 halfSize( (maxX - minX)/2.0f, (maxY - minY)/2.0f, (maxZ - minZ)/2.0f );
    Vector3 center( minX + halfSize.x, minY + halfSize.y, minZ + halfSize.z );
    newBounds.Set( center, halfSize );

    // Only resize if the bounds changed.
    if( newBounds != m_pRootNode->m_Bounds )
    {
        //LOGInfo( LOGTag, "Recentering octree (%0.2f, %0.2f, %0.2f)\n", center.x, center.y, center.z );

        CollapseChildNodes( m_pRootNode );

        m_pRootNode->m_Bounds.Set( center, halfSize );

        UpdateTree( m_pRootNode );
    }
}

SceneGraphObject* SceneGraph_Octree::AddObjectWithFlagOverride(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, MyRE::PrimitiveTypes primitiveType, int pointSize, SceneGraphFlags flags, unsigned int layers, void* pUserData)
{
    //LOGInfo( "SceneGraph", "Add object %d\n", pUserData );

    MyAssert( pTransform != nullptr );

    SceneGraphObject* pObject = m_pObjectPool.GetObjectFromPool();

    if( pObject )
    {
        pObject->Clear();

        pObject->SetFlags( flags );
        pObject->SetMaterial( pMaterial, false );

        pObject->m_Layers = layers;

        pObject->m_pTransform = pTransform;
        pObject->m_pMesh = pMesh;
        pObject->m_pSubmesh = pSubmesh;
        pObject->m_Visible = true;

        pObject->m_GLPrimitiveType = primitiveType;
        pObject->m_PointSize = pointSize;

        pObject->m_pUserData = pUserData;

        m_pRootNode->m_Renderables.AddTail( pObject );
    }
    else
    {
        LOGInfo( "Scene Graph", "Not enough renderable objects in list\n" );
    }

    m_Dirty = true;

    return pObject;
}

void SceneGraph_Octree::RemoveObject(SceneGraphObject* pObject)
{
    //LOGInfo( "SceneGraph", "Remove object %d\n", pObject->m_pUserData );

    MyAssert( pObject != nullptr );

    MyAssert( pObject->Next != nullptr );
    pObject->Remove();

    pObject->Clear();
    m_pObjectPool.ReturnObjectToPool( pObject );
}

void SceneGraph_Octree::ObjectMoved(SceneGraphObject* pObject)
{
    // Move any object that moves to the root of the octree.
    // TODO: Better.
    m_pRootNode->m_Renderables.MoveTail( pObject );
}

void SceneGraph_Octree::Draw(bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr* pPreDrawCallbackFunc)
{
    if( m_Dirty )
    {
        UpdateTree( m_pRootNode );
    }

    DrawNode( m_pRootNode, drawOpaques, emissiveDrawOption, layersToRender, camPos, camRot, pMatProj, pMatView, shadowlightVP, pShadowTex, pShaderOverride, pPreDrawCallbackFunc );
}

void SceneGraph_Octree::DrawNode(OctreeNode* pOctreeNode, bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr* pPreDrawCallbackFunc)
{
    // Draw all scene graph objects contained in this node.

    // If node is not in frustum and it's not the root node, return.
    if( pOctreeNode != m_pRootNode && FitsInFrustum( &pOctreeNode->m_Bounds, pMatProj, pMatView, nullptr ) == false )
        return;

    for( SceneGraphObject* pObject = pOctreeNode->m_Renderables.GetHead(); pObject; pObject = pObject->GetNext() )
    {
        // Skip object if it doesn't match transparency/emissive settings, isn't on the right layer, etc.
        if( ShouldObjectBeDrawn( pObject, drawOpaques, emissiveDrawOption, layersToRender ) == false )
            continue;

        // Pull info from SceneGraphObject.
        MyMatrix worldtransform = *pObject->m_pTransform;
        MyMesh* pMesh = pObject->m_pMesh;

        // Simple frustum check. // Removed since entire octree nodes will be culled, but this could still be useful.
        //if( pMesh != nullptr ) // TODO: Particle Renderers don't have a mesh, so no bounds and won't get frustum culled.
        //{
        //    MyAABounds* bounds = pMesh->GetBounds();

        //    if( FitsInFrustum( bounds, pMatProj, pMatView, &worldtransform ) == false )
        //        continue;
        //}

        // Update submesh with material, etc from SceneGraphObject.
        MySubmesh* pSubmesh = pObject->m_pSubmesh;
        MaterialDefinition* pMaterial = pObject->GetMaterial();
        pSubmesh->SetMaterial( pMaterial );
        pSubmesh->m_PrimitiveType = pObject->m_GLPrimitiveType;
        pSubmesh->m_PointSize = pObject->m_PointSize;

        // Find nearest lights.
        MyLight* lights[5];
        int numlights = g_pLightManager->FindNearestLights( LightType_Directional, 1, worldtransform.GetTranslation(), &lights[0] );
        numlights += g_pLightManager->FindNearestLights( LightType_Point, 4, worldtransform.GetTranslation(), &lights[numlights] );

        if( pPreDrawCallbackFunc )
        {
            (*pPreDrawCallbackFunc)( pObject, pShaderOverride );
        }

        if( pMesh )
        {
            pMesh->PreDraw();
        }

#if MYFW_EDITOR
        bool hideFromDrawList = pObject->IsEditorObject();
#else
        bool hideFromDrawList = false;
#endif

#if MYFW_EDITOR
        if( pMaterial == nullptr )
        {
            pMaterial = m_pGameCore->GetManagers()->GetMaterialManager()->GetDefaultEditorMaterial();
        }
#endif

        pSubmesh->Draw( pMaterial, pMesh, pMatProj, pMatView, &worldtransform, camPos, camRot, lights, numlights, shadowlightVP, pShadowTex, nullptr, pShaderOverride, hideFromDrawList );
    }

    // Recurse through children.
    for( int i=0; i<8; i++ )
    {
        if( pOctreeNode->m_pChildNodes[i] != nullptr )
        {
            DrawNode( pOctreeNode->m_pChildNodes[i], drawOpaques, emissiveDrawOption, layersToRender, camPos, camRot, pMatProj, pMatView, shadowlightVP, pShadowTex, pShaderOverride, pPreDrawCallbackFunc );
        }
    }
}
