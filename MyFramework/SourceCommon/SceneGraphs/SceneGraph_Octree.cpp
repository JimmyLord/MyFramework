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

    m_NumRenderables = 0;
}

OctreeNode::~OctreeNode()
{
    while( m_Renderables.GetHead() )
    {
        m_pSceneGraph->m_pObjectPool.ReturnObjectToPool( (SceneGraphObject*)m_Renderables.RemHead() );

        m_NumRenderables--;
    }

    for( int i=0; i<8; i++ )
    {
        m_pSceneGraph->m_NodePool.ReturnObjectToPool( m_pChildNodes[i] );
    }

}

SceneGraph_Octree::SceneGraph_Octree(unsigned int treedepth, int minx, int miny, int minz, int maxx, int maxy, int maxz)
{
    m_MaxDepth = treedepth;

    int maxnodes = 0;
    int numatdepth = 1;
    for( unsigned int i=0; i<treedepth; i++ )
    {
        maxnodes += numatdepth;
        numatdepth *= 8;
    }

    m_NodePool.AllocateObjects( maxnodes );

    m_RootNode = m_NodePool.GetObjectFromPool();

    Vector3 halfsize( (minx + maxx)/2.0f, (miny + maxy)/2.0f, (minz + maxz)/2.0f );
    Vector3 center( minx + halfsize.x, miny + halfsize.y, minz + halfsize.z );

    m_RootNode->m_Bounds.Set( center, halfsize );
    m_RootNode->m_pSceneGraph = this;
}

SceneGraph_Octree::~SceneGraph_Octree()
{
    m_NodePool.ReturnObjectToPool( m_RootNode );
}

bool FitsInsideAABB(MyAABounds* pOuterBounds, MyAABounds* pInnerBounds)
{
    return false;
}

void SceneGraph_Octree::UpdateTree(OctreeNode* pOctreeNode)
{
    // move all objects in pOctreeNode node down as far as they can go.
    for( CPPListNode* pNode = pOctreeNode->m_Renderables.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        SceneGraphObject* pObject = (SceneGraphObject*)pNode;
        MyAABounds* meshbounds = pObject->m_pMesh->GetBounds();

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

            if( FitsInsideAABB( &childbounds, meshbounds ) )
            {
                if( pOctreeNode->m_pChildNodes[i] == 0 )
                {
                    pOctreeNode->m_pChildNodes[i] = m_NodePool.GetObjectFromPool();
                    pOctreeNode->m_pChildNodes[i]->m_Bounds = childbounds;
                    pOctreeNode->m_pChildNodes[i]->m_pParentNode = pOctreeNode;
                    pOctreeNode->m_pChildNodes[i]->m_pSceneGraph = this;
                }

                pOctreeNode->m_pChildNodes[i]->m_Renderables.MoveTail( pObject );
                pOctreeNode->m_NumRenderables--;
                pOctreeNode->m_pChildNodes[i]->m_NumRenderables++;
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

        m_RootNode->m_Renderables.AddTail( pObject );
        m_RootNode->m_NumRenderables++;
    }
    else
    {
        LOGInfo( "Scene Graph", "Not enough renderable objects in list\n" );
    }

    UpdateTree( m_RootNode );

    return pObject;
}

void SceneGraph_Octree::RemoveObject(SceneGraphObject* pObject)
{
    //LOGInfo( "SceneGraph", "Remove object %d\n", pObject->m_pUserData );

    MyAssert( pObject != 0 );

    pObject->Remove();
    m_RootNode->m_NumRenderables--;

    m_pObjectPool.ReturnObjectToPool( pObject );
}

void SceneGraph_Octree::Draw(SceneGraphFlags flags, unsigned int layerstorender, Vector3* campos, Vector3* camrot, MyMatrix* pMatViewProj, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr pPreDrawCallbackFunc)
{
    checkGlError( "Start of SceneGraph_Octree::Draw()" );

    for( CPPListNode* pNode = m_RootNode->m_Renderables.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        SceneGraphObject* pObject = (SceneGraphObject*)pNode;

        if( (pObject->m_Flags & flags) == 0 )
            continue;

        if( (pObject->m_Layers & layerstorender) == 0 )
            continue;
        
        MyAssert( pObject->m_pSubmesh );
        //MyAssert( pObject->m_pMaterial );

        if( pObject->m_pSubmesh == 0 || pObject->m_pMaterial == 0 )
            continue;

        if( pObject->m_Visible == false )
            continue;

        MyMatrix worldtransform = *pObject->m_pTransform;
        MyMesh* pMesh = pObject->m_pMesh;
        MySubmesh* pSubmesh = pObject->m_pSubmesh;
        MaterialDefinition* pMaterial = pObject->m_pMaterial;

        // simple frustum check
        if( pMesh != 0 ) // TODO: Particle Renderers don't have a mesh, so no bounds and won't get frustum culled
        {
            MyAABounds* bounds = pMesh->GetBounds();
            Vector3 center = bounds->GetCenter();
            Vector3 half = bounds->GetHalfSize();

            MyMatrix wvp = *pMatViewProj * worldtransform;

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
                continue;
        }

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
}
