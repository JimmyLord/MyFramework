//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "Box2DDebugDraw.h"

Box2DDebugDraw::Box2DDebugDraw(MaterialDefinition* debugdrawmaterial, MyMatrix* matviewproj)
{
    m_pMatViewProj = matviewproj;

    m_pMesh = MyNew MyMesh();
    m_pMesh->CreateSubmeshes( 1 );
    m_pMesh->SetMaterial( debugdrawmaterial, 0 );

    m_pVertexFormatDesc = MyNew VertexFormat_Dynamic_Desc; 
    m_pVertexFormatDesc->stride = sizeof(float) * 2; // just xy
    m_pVertexFormatDesc->num_position_components = 2;
    m_pVertexFormatDesc->offset_pos = 0;

    m_pMesh->m_SubmeshList[0]->m_VertexFormat = VertexFormat_XYZ;
    m_pMesh->m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer();
    m_pMesh->m_SubmeshList[0]->m_pVertexBuffer->InitializeBuffer(
        0, 0, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, false, 1, VertexFormat_Dynamic, m_pVertexFormatDesc, "Box2DDebugDraw", "Verts" );
}

Box2DDebugDraw::~Box2DDebugDraw()
{
    SAFE_RELEASE( m_pMesh );
    SAFE_DELETE( m_pVertexFormatDesc );
}

void Box2DDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
}

void Box2DDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    m_pMesh->m_SubmeshList[0]->m_pVertexBuffer->m_pData = (char*)vertices;
    m_pMesh->m_SubmeshList[0]->m_pVertexBuffer->Rebuild( 0, vertexCount * sizeof(float)*2, true );
    m_pMesh->m_SubmeshList[0]->m_pVertexBuffer->m_pData = 0;
    m_pMesh->m_SubmeshList[0]->m_NumVertsToDraw = (unsigned short)vertexCount;
    m_pMesh->m_SubmeshList[0]->m_PrimitiveType = GL_TRIANGLE_FAN;
    m_pMesh->m_MeshReady = true;

    MaterialDefinition* pMat = m_pMesh->GetMaterial( 0 );
    pMat->SetColorDiffuse( ColorByte( (unsigned char)(color.r*255), (unsigned char)(color.g*255), (unsigned char)(color.b*255), 128 ) );

    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );

    m_pMesh->Draw( m_pMatViewProj, 0, 0, 0, 0, 0, 0, 0 );

    pMat->SetColorDiffuse( ColorByte( (unsigned char)(color.r*255), (unsigned char)(color.g*255), (unsigned char)(color.b*255), 255 ) );
    m_pMesh->m_SubmeshList[0]->m_PrimitiveType = GL_LINE_LOOP;
    glLineWidth( 1 );
    m_pMesh->Draw( m_pMatViewProj, 0, 0, 0, 0, 0, 0, 0 );

    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );
}

void Box2DDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
}

void Box2DDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
}

void Box2DDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
}

void Box2DDebugDraw::DrawTransform(const b2Transform& xf)
{
}
