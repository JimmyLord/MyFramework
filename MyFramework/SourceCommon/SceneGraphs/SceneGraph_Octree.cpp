//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "SceneGraph_Base.h"
#include "SceneGraph_Octree.h"

OctreeNode::OctreeNode()
{
    m_pSceneGraph = 0;

    for( int i=0; i<8; i++ )
    {
        m_pChildNodes[i] = 0;
    }
    m_pParentNode = 0;
}

OctreeNode::~OctreeNode()
{
    MyAssert( m_pSceneGraph == 0 );
    MyAssert( m_pParentNode == 0 );    
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
            m_pChildNodes[i] = 0;
        }
    }

    m_pSceneGraph->m_NodePool.ReturnObjectToPool( this );
    m_pSceneGraph = 0;
    m_pParentNode = 0;
}

SceneGraph_Octree::SceneGraph_Octree(unsigned int treedepth, float minx, float miny, float minz, float maxx, float maxy, float maxz)
{
    m_MaxDepth = treedepth;

    int maxnodes = 0;
    int numatdepth = 1;
    for( unsigned int i=0; i<treedepth; i++ )
    {
        maxnodes += numatdepth;
        numatdepth *= 8;
    }

    m_Dirty = false;

    m_NodePool.AllocateObjects( maxnodes );

    m_pRootNode = m_NodePool.GetObjectFromPool();

    Vector3 halfsize( (maxx - minx)/2.0f, (maxy - miny)/2.0f, (maxz - minz)/2.0f );
    Vector3 center( minx + halfsize.x, miny + halfsize.y, minz + halfsize.z );

    m_pRootNode->m_Bounds.Set( center, halfsize );
    m_pRootNode->m_pSceneGraph = this;
    m_pRootNode->m_NodeDepth = 0;
}

SceneGraph_Octree::~SceneGraph_Octree()
{
    m_pRootNode->Cleanup();
}

bool FitsInsideAABB(MyAABounds* pOuterBounds, MyAABounds* pInnerBounds, Vector3 innerOffset)
{
    Vector3 outercenter = pOuterBounds->GetCenter();
    Vector3 outerhalfsize = pOuterBounds->GetHalfSize();
    Vector3 innercenter = pInnerBounds->GetCenter();
    Vector3 innerhalfsize = pInnerBounds->GetHalfSize();

    Vector3 outermin = outercenter - outerhalfsize;
    Vector3 outermax = outercenter + outerhalfsize;
    Vector3 innermin = innercenter - innerhalfsize + innerOffset;
    Vector3 innermax = innercenter + innerhalfsize + innerOffset;
    
    if( innermin.x < outermin.x ) return false;
    if( innermin.y < outermin.y ) return false;
    if( innermin.z < outermin.z ) return false;
    if( innermax.x > outermax.x ) return false;
    if( innermax.y > outermax.y ) return false;
    if( innermax.z > outermax.z ) return false;

    return true;
}

bool FitsInFrustum(MyAABounds* pBounds, MyMatrix* pMatViewProj, MyMatrix* pWorldTransform)
{
    Vector3 center = pBounds->GetCenter();
    Vector3 half = pBounds->GetHalfSize();

    // create a wvp matrix for pBounds
    MyMatrix wvp = *pMatViewProj;    
    if( pWorldTransform )
        wvp = wvp * *pWorldTransform;

    Vector4 clippos[8];

    // transform AABB extents into clip space.
    clippos[0] = wvp * Vector4(center.x - half.x, center.y - half.y, center.z - half.z, 1);
    clippos[1] = wvp * Vector4(center.x - half.x, center.y - half.y, center.z + half.z, 1);
    clippos[2] = wvp * Vector4(center.x - half.x, center.y + half.y, center.z - half.z, 1);
    clippos[3] = wvp * Vector4(center.x - half.x, center.y + half.y, center.z + half.z, 1);
    clippos[4] = wvp * Vector4(center.x + half.x, center.y - half.y, center.z - half.z, 1);
    clippos[5] = wvp * Vector4(center.x + half.x, center.y - half.y, center.z + half.z, 1);
    clippos[6] = wvp * Vector4(center.x + half.x, center.y + half.y, center.z - half.z, 1);
    clippos[7] = wvp * Vector4(center.x + half.x, center.y + half.y, center.z + half.z, 1);

    // check visibility two planes at a time
    bool visible;
    for( int component=0; component<3; component++ ) // loop through x/y/z
    {
        // check if all 8 points are less than the -w extent of it's axis
        visible = false;
        for( int i=0; i<8; i++ )
        {
            if( clippos[i][component] >= -clippos[i].w )
            {
                visible = true; // this point is on the visible side of the plane, skip to next plane
                break;
            }
        }
        if( visible == false ) // all points are on outside of plane, don't draw object
            break;

        // check if all 8 points are greater than the -w extent of it's axis
        visible = false;
        for( int i=0; i<8; i++ )
        {
            if( clippos[i][component] <= clippos[i].w )
            {
                visible = true; // this point is on the visible side of the plane, skip to next plane
                break;
            }
        }
        if( visible == false ) // all points are on outside of plane, don't draw object
            break;
    }

    // if all points are on outside of frustum, don't draw mesh.
    if( visible == false )
        return false;

    return true;
}

void SceneGraph_Octree::UpdateTree(OctreeNode* pOctreeNode)
{
    if( pOctreeNode->m_NodeDepth >= m_MaxDepth - 1 )
        return;

    // move all objects in pOctreeNode node down as far as they can go.
    CPPListNode* pNextNode;
    for( CPPListNode* pNode = pOctreeNode->m_Renderables.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        SceneGraphObject* pObject = (SceneGraphObject*)pNode;
        
        if( pObject->m_pMesh == 0 )
            continue;

        MyAABounds* meshbounds = pObject->m_pMesh->GetBounds();
        Vector3 meshpos = pObject->m_pTransform->GetTranslation();

        for( int i=0; i<8; i++ )
        {
            MyAABounds childbounds;

            if( pOctreeNode->m_pChildNodes[i] == 0 )
            {
                MyAABounds* currentbounds = &pOctreeNode->m_Bounds;
                Vector3 center = currentbounds->GetCenter();
                Vector3 quartersize = currentbounds->GetHalfSize()/2;

                if( i == 0 ) childbounds.Set( Vector3( center.x - quartersize.x, center.y + quartersize.y, center.z + quartersize.z ), quartersize );
                if( i == 1 ) childbounds.Set( Vector3( center.x + quartersize.x, center.y + quartersize.y, center.z + quartersize.z ), quartersize );
                if( i == 2 ) childbounds.Set( Vector3( center.x - quartersize.x, center.y + quartersize.y, center.z - quartersize.z ), quartersize );
                if( i == 3 ) childbounds.Set( Vector3( center.x + quartersize.x, center.y + quartersize.y, center.z - quartersize.z ), quartersize );
                if( i == 4 ) childbounds.Set( Vector3( center.x - quartersize.x, center.y - quartersize.y, center.z + quartersize.z ), quartersize );
                if( i == 5 ) childbounds.Set( Vector3( center.x + quartersize.x, center.y - quartersize.y, center.z + quartersize.z ), quartersize );
                if( i == 6 ) childbounds.Set( Vector3( center.x - quartersize.x, center.y - quartersize.y, center.z - quartersize.z ), quartersize );
                if( i == 7 ) childbounds.Set( Vector3( center.x + quartersize.x, center.y - quartersize.y, center.z - quartersize.z ), quartersize );
            }
            else
            {
                childbounds = pOctreeNode->m_pChildNodes[i]->m_Bounds;
            }

            if( FitsInsideAABB( &childbounds, meshbounds, meshpos ) )
            {
                if( pOctreeNode->m_pChildNodes[i] == 0 )
                {
                    pOctreeNode->m_pChildNodes[i] = m_NodePool.GetObjectFromPool();
                    pOctreeNode->m_pChildNodes[i]->m_Bounds = childbounds;
                    pOctreeNode->m_pChildNodes[i]->m_pParentNode = pOctreeNode;
                    pOctreeNode->m_pChildNodes[i]->m_pSceneGraph = this;
                    pOctreeNode->m_pChildNodes[i]->m_NodeDepth = pOctreeNode->m_NodeDepth + 1;
                }

                pOctreeNode->m_pChildNodes[i]->m_Renderables.MoveTail( pObject );
            }
        }
    }

    // recurse through children
    for( int i=0; i<8; i++ )
    {
        if( pOctreeNode->m_pChildNodes[i] != 0 )
        {
            UpdateTree( pOctreeNode->m_pChildNodes[i] );
        }
    }
}

SceneGraphObject* SceneGraph_Octree::AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitive, int pointsize, SceneGraphFlags flags, unsigned int layers, void* pUserData)
{
    //LOGInfo( "SceneGraph", "Add object %d\n", pUserData );

    MyAssert( pTransform != 0 );

    SceneGraphObject* pObject = m_pObjectPool.GetObjectFromPool();

    if( pObject )
    {
        pObject->m_Flags = flags;
        pObject->m_Layers = layers;

        pObject->m_pTransform = pTransform;
        pObject->m_pMesh = pMesh;
        pObject->m_pSubmesh = pSubmesh;
        pObject->m_pMaterial = pMaterial;
        pObject->m_Visible = true;

        pObject->m_GLPrimitiveType = primitive;
        pObject->m_PointSize = pointsize;

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

    MyAssert( pObject != 0 );

    MyAssert( pObject->Next != 0 );
    pObject->Remove();

    m_pObjectPool.ReturnObjectToPool( pObject );
}

void SceneGraph_Octree::Draw(SceneGraphFlags flags, unsigned int layerstorender, Vector3* campos, Vector3* camrot, MyMatrix* pMatViewProj, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr pPreDrawCallbackFunc)
{
    checkGlError( "Start of SceneGraph_Octree::Draw()" );

    if( m_Dirty )
    {
        UpdateTree( m_pRootNode );
    }

    DrawNode( m_pRootNode, flags, layerstorender, campos, camrot, pMatViewProj, shadowlightVP, pShadowTex, pShaderOverride, pPreDrawCallbackFunc );
}

void SceneGraph_Octree::DrawNode(OctreeNode* pOctreeNode, SceneGraphFlags flags, unsigned int layerstorender, Vector3* campos, Vector3* camrot, MyMatrix* pMatViewProj, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr pPreDrawCallbackFunc)
{
    // Draw all scene graph objects contained in this node.
    // TODO:
    //    Remove frustum check for each individual object

    // If node is not in frustum, return
    if( FitsInFrustum( &pOctreeNode->m_Bounds, pMatViewProj, 0 ) == false )
        return;

    for( CPPListNode* pNode = pOctreeNode->m_Renderables.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        SceneGraphObject* pObject = (SceneGraphObject*)pNode;

        if( (pObject->m_Flags & flags) == 0 )
            continue;

        if( (pObject->m_Layers & layerstorender) == 0 )
            continue;
        
        MyAssert( pObject->m_pSubmesh );
        //MyAssert( pObject->m_pMaterial );

        if( pObject->m_pSubmesh == 0 ) //|| pObject->m_pMaterial == 0 )
            continue;

        if( pObject->m_Visible == false )
            continue;

        MyMatrix worldtransform = *pObject->m_pTransform;
        MyMesh* pMesh = pObject->m_pMesh;
        MySubmesh* pSubmesh = pObject->m_pSubmesh;
        MaterialDefinition* pMaterial = pObject->m_pMaterial;

        // simple frustum check
        //if( pMesh != 0 ) // TODO: Particle Renderers don't have a mesh, so no bounds and won't get frustum culled
        //{
        //    MyAABounds* bounds = pMesh->GetBounds();

        //    if( FitsInFrustum( bounds, pMatViewProj, &worldtransform ) == false )
        //        continue;
        //}

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

        checkGlError( "SceneGraph_Octree::Draw() before pSubmesh->Draw()" );

        pSubmesh->Draw( pMesh, &worldtransform, pMatViewProj, campos, camrot, lights, numlights, shadowlightVP, pShadowTex, 0, pShaderOverride );

        checkGlError( "SceneGraph_Octree::Draw() after pSubmesh->Draw()" );
    }

    checkGlError( "End of SceneGraph_Octree::Draw()" );

    // recurse through children
    for( int i=0; i<8; i++ )
    {
        if( pOctreeNode->m_pChildNodes[i] != 0 )
        {
            DrawNode( pOctreeNode->m_pChildNodes[i], flags, layerstorender, campos, camrot, pMatViewProj, shadowlightVP, pShadowTex, pShaderOverride, pPreDrawCallbackFunc );
        }
    }
}
