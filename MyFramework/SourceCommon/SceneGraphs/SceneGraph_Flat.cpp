//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "SceneGraph_Base.h"
#include "SceneGraph_Flat.h"

SceneGraph_Flat::SceneGraph_Flat()
{
    m_NumRenderables = 0;
}

SceneGraph_Flat::~SceneGraph_Flat()
{
    for( unsigned int i=0; i<m_NumRenderables; i++ )
    {
        m_pObjectPool.ReturnObject( m_pRenderables[i] );
    }
}

SceneGraphObject* SceneGraph_Flat::AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitive, int pointsize, SceneGraphFlags flags)
{
    MyAssert( pTransform != 0 );

    SceneGraphObject* pObject = m_pObjectPool.GetObject();

    if( pObject )
    {
        MyAssert( m_NumRenderables < 10000 );

        pObject->m_Flags = flags;

        pObject->m_pTransform = pTransform;
        pObject->m_pMesh = pMesh;
        pObject->m_pSubmesh = pSubmesh;
        pObject->m_pMaterial = pMaterial;
        pObject->m_Visible = true;

        pObject->m_GLPrimitiveType = primitive;
        pObject->m_PointSize = pointsize;

        m_pRenderables[m_NumRenderables] = pObject;
        m_NumRenderables++;
    }
    else
    {
        LOGInfo( "Scene Graph", "Not enough renderable objects in list\n" );
    }

    return pObject;
}

void SceneGraph_Flat::RemoveObject(SceneGraphObject* pObject)
{
    for( unsigned int i=0; i<m_NumRenderables; i++ )
    {
        if( m_pRenderables[i] == pObject )
        {
            m_pRenderables[i] = m_pRenderables[m_NumRenderables-1];
            m_NumRenderables--;
            break;
        }
    }

    m_pObjectPool.ReturnObject( pObject );
}

void SceneGraph_Flat::Draw(SceneGraphFlags flags, Vector3* campos, Vector3* camrot, MyMatrix* pMatViewProj, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride)
{
    for( unsigned int i=0; i<m_NumRenderables; i++ )
    {
        SceneGraphObject* pObject = m_pRenderables[i];

        if( (pObject->m_Flags & flags) == 0 )
            continue;
        
        MyAssert( pObject->m_pSubmesh );
        MyAssert( pObject->m_pMaterial );

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
        MyLight* lights;
        int numlights = g_pLightManager->FindNearestLights( 4, worldtransform.GetTranslation(), &lights );

        pSubmesh->Draw( pMesh, &worldtransform, pMatViewProj, campos, camrot, lights, numlights, shadowlightVP, pShadowTex, 0, pShaderOverride );
    }
}