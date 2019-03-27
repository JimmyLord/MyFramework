//
// Copyright (c) 2016-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "SceneGraph_Base.h"
#include "SceneGraph_Flat.h"
#include "../Core/GameCore.h"
#include "../DataTypes/MyAABounds.h"
#include "../DataTypes/MyActivePool.h"
#include "../Meshes/LightManager.h"
#include "../Meshes/MyMesh.h"
#include "../Meshes/MySubmesh.h"
#include "../Textures/MaterialManager.h"

SceneGraph_Flat::SceneGraph_Flat(GameCore* pGameCore)
: SceneGraph_Base( pGameCore )
{
    m_NumRenderables = 0;
}

SceneGraph_Flat::~SceneGraph_Flat()
{
    while( m_Renderables.GetHead() )
    {
        m_pObjectPool.ReturnObjectToPool( (SceneGraphObject*)m_Renderables.RemHead() );

        m_NumRenderables--;
    }

    MyAssert( m_NumRenderables == 0 );
}

SceneGraphObject* SceneGraph_Flat::AddObjectWithFlagOverride(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, MyRE::PrimitiveTypes primitiveType, int pointSize, SceneGraphFlags flags, unsigned int layers, void* pUserData)
{
    //LOGInfo( "SceneGraph", "Add object %d\n", pUserData );

    MyAssert( pTransform != nullptr );

    SceneGraphObject* pObject = m_pObjectPool.GetObjectFromPool();

    if( pObject )
    {
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

        m_Renderables.AddTail( pObject );

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
    //LOGInfo( "SceneGraph", "Remove object %d\n", pObject->m_pUserData );

    MyAssert( pObject != nullptr );

    pObject->Remove();
    m_NumRenderables--;

    pObject->Clear();
    m_pObjectPool.ReturnObjectToPool( pObject );
}

void SceneGraph_Flat::ObjectMoved(SceneGraphObject* pObject)
{
}

void SceneGraph_Flat::Draw(bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr* pPreDrawCallbackFunc)
{
    MyAssert( pMatProj != nullptr );
    MyAssert( pMatView != nullptr );

    MyMatrix matViewProj = *pMatProj * *pMatView;

    for( SceneGraphObject* pObject = m_Renderables.GetHead(); pObject; pObject = pObject->GetNext() )
    {
        // Skip object if it doesn't match transparency/emissive settings, isn't on the right layer, etc.
        if( ShouldObjectBeDrawn( pObject, drawOpaques, emissiveDrawOption, layersToRender ) == false )
            continue;

        MyMatrix worldTransform = *pObject->m_pTransform;
        MyMesh* pMesh = pObject->m_pMesh;
        MySubmesh* pSubmesh = pObject->m_pSubmesh;
        MaterialDefinition* pMaterial = pObject->GetMaterial();

        // Simple frustum check.
        if( pMesh != nullptr ) // TODO: Particle Renderers don't have a mesh, so no bounds and won't get frustum culled.
        {
            MyAABounds* bounds = pMesh->GetBounds();
            Vector3 center = bounds->GetCenter();
            Vector3 half = bounds->GetHalfSize();

            MyMatrix wvp = matViewProj * worldTransform;

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
            bool visible;
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

                // Check if all 8 points are greater than the -w extent of it's axis.
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
                continue;
        }

        pSubmesh->SetMaterial( pMaterial );
        pSubmesh->m_PrimitiveType = pObject->m_GLPrimitiveType;
        pSubmesh->m_PointSize = pObject->m_PointSize;

        // Find nearest lights.
        MyLight* lights[5];
        int numLights = g_pLightManager->FindNearestLights( LightType_Directional, 1, worldTransform.GetTranslation(), &lights[0] );
        numLights += g_pLightManager->FindNearestLights( LightType_Point, 4, worldTransform.GetTranslation(), &lights[numLights] );

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

        pSubmesh->Draw( pMaterial, pMesh, pMatProj, pMatView, &worldTransform, camPos, camRot, lights, numLights, shadowlightVP, pShadowTex, nullptr, pShaderOverride, hideFromDrawList );
    }
}
