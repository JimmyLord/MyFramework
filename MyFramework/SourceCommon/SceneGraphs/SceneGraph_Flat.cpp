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

SceneGraphObject* SceneGraph_Flat::AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial)
{
    MyAssert( pTransform != 0 && pMesh != 0 );

    SceneGraphObject* pObject = m_pObjectPool.GetObject();

    if( pObject )
    {
        MyAssert( m_NumRenderables < 10000 );

        pObject->m_pTransform = pTransform;
        pObject->m_pMesh = pMesh;
        pObject->m_pSubmesh = pSubmesh;
        pObject->m_pMaterial = pMaterial;

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

void SceneGraph_Flat::Draw(Vector3 campos, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    for( unsigned int i=0; i<m_NumRenderables; i++ )
    {
        SceneGraphObject* pObject = m_pRenderables[i];
        MyAssert( pObject->m_pMesh );
        MyAssert( pObject->m_pSubmesh );
        MyAssert( pObject->m_pMaterial );

        if( pObject->m_pMesh && pObject->m_pSubmesh && pObject->m_pMaterial )
        {
            MyMatrix worldtransform = *pObject->m_pTransform;//*m_pComponentTransform->GetWorldTransform();
            MyMesh* pMesh = pObject->m_pMesh;
            MySubmesh* pSubmesh = pObject->m_pSubmesh;
            MaterialDefinition* pMaterial = pObject->m_pMaterial;

            // simple frustum check
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
                    return;
            }

            for( unsigned int i=0; i<pMesh->m_SubmeshList.Count(); i++ )
            {
                pSubmesh->SetMaterial( pMaterial );
                // TODO: fix
                //pSubmesh->m_PrimitiveType = m_GLPrimitiveType;
                //pSubmesh->m_PointSize = m_PointSize;
            }

            //m_pMesh->SetTransform( worldtransform );

            // Find nearest lights.
            MyLight* lights;
            int numlights = g_pLightManager->FindNearestLights( 4, worldtransform.GetTranslation(), &lights );

            // TODO: fix
            // Find nearest shadow casting light.
    //        MyMatrix* pShadowVP = 0;
    //        TextureDefinition* pShadowTex = 0;
    //        if( g_ActiveShaderPass == ShaderPass_Main )
    //        {
    //            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByName( "Shadow Light" );
    //            if( pObject )
    //            {
    //                ComponentBase* pComponent = pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera );
    //                ComponentCameraShadow* pShadowCam = pComponent->IsA( "CameraShadowComponent" ) ? (ComponentCameraShadow*)pComponent : 0;
    //                if( pShadowCam )
    //                {
    //                    pShadowVP = &pShadowCam->m_matViewProj;
    //#if 1
    //                    pShadowTex = pShadowCam->m_pDepthFBO->m_pDepthTexture;
    //#else
    //                    pShadowTex = pShadowCam->m_pDepthFBO->m_pColorTexture;
    //#endif
    //                }
    //            }
    //        }

    //        Vector3 campos;
    //#if MYFW_USING_WX
    //        if( g_pEngineCore->m_EditorMode )
    //        {
    //            ComponentCamera* pCamera = g_pEngineCore->m_pEditorState->GetEditorCamera();

    //            campos = pCamera->m_pComponentTransform->GetLocalPosition();
    //        }
    //        else
    //#endif
    //        {
    //            ComponentCamera* pCamera = g_pComponentSystemManager->GetFirstCamera();
    //            if( pCamera )
    //            {
    //                campos = pCamera->m_pComponentTransform->GetLocalPosition();
    //            }
    //            else
    //            {
    //                campos.Set( 0, 0, 0 );
    //            }
    //        }

            // TODO: fix shadows.
            //pSubmesh->Draw( pMesh, &worldtransform, pMatViewProj, &campos, lights, numlights, pShadowVP, pShadowTex, 0, pShaderOverride );
            pSubmesh->Draw( pMesh, &worldtransform, pMatViewProj, &campos, lights, numlights, 0, 0, 0, pShaderOverride );
        }
    }
}
