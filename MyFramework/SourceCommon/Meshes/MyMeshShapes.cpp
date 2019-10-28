//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "BufferManager.h"
#include "MyMesh.h"
#include "MySubmesh.h"
#include "../Core/GameCore.h"
#include "../DataTypes/ColorStructs.h"
#include "../DataTypes/MyAABounds.h"
#include "../Renderers/BaseClasses/Renderer_Enums.h"
#include "../Renderers/BaseClasses/Renderer_Base.h"

//============================================================================================================================
// Shape creation functions.  Declared in MyMesh.h
//============================================================================================================================
void MyMesh::CreateShapeBuffers(bool createIndexBuffer)
{
    if( m_SubmeshList.Length() == 0 )
    {
        CreateSubmeshes( 1 );
    }
    MyAssert( m_SubmeshList.Count() == 1 );

    if( m_SubmeshList[0]->m_pVertexBuffer == nullptr )
    {
        BufferManager* pBufferManager = m_pGameCore->GetManagers()->GetBufferManager();
        m_SubmeshList[0]->m_pVertexBuffer = pBufferManager->CreateBuffer();
    }

    if( createIndexBuffer && m_SubmeshList[0]->m_pIndexBuffer == nullptr )
    {
        BufferManager* pBufferManager = m_pGameCore->GetManagers()->GetBufferManager();
        m_SubmeshList[0]->m_pIndexBuffer = pBufferManager->CreateBuffer();
    }
}

void MyMesh::RebuildShapeBuffers(uint32 numVerts, VertexFormats vertexFormat, MyRE::PrimitiveTypes primitiveType, uint32 numIndices, MyRE::IndexTypes indexType, const char* category)
{
    CreateShapeBuffers( numIndices > 0 ? true : false );

    m_SubmeshList[0]->m_NumVertsToDraw = numVerts;
    m_SubmeshList[0]->m_NumIndicesToDraw = numIndices;
    m_SubmeshList[0]->m_VertexFormat = vertexFormat;
    m_SubmeshList[0]->m_PrimitiveType = primitiveType;

    // Rebuild the vertex buffer.
    {
        unsigned int bufferSize = g_VertexFormatSizes[vertexFormat] * numVerts;

        void* pVerts = m_SubmeshList[0]->m_pVertexBuffer->GetData( true );

        // Delete and rebuild the old buffer if its not the same size.
        if( bufferSize != m_SubmeshList[0]->m_pVertexBuffer->GetDataSize() )
        {
            m_SubmeshList[0]->m_pVertexBuffer->FreeBufferedData();
            pVerts = MyNew char[bufferSize];
        }

        m_SubmeshList[0]->m_pVertexBuffer->InitializeBuffer( pVerts, bufferSize, MyRE::BufferType_Vertex, MyRE::BufferUsage_StaticDraw,
                                                             false, 1, vertexFormat, nullptr, category, "Verts" );
    }

    // Rebuild the index buffer.
    if( numIndices > 0 )
    {
        int bytesPerIndex = MyRE::IndexTypeByteSizes[indexType];

        unsigned int bufferSize = bytesPerIndex*numIndices;

        void* pIndices = m_SubmeshList[0]->m_pIndexBuffer->GetData( true );

        // Delete and rebuild the old buffer if its not the same size.
        if( bytesPerIndex*numIndices != m_SubmeshList[0]->m_pIndexBuffer->GetDataSize() )
        {
            m_SubmeshList[0]->m_pIndexBuffer->FreeBufferedData();
            pIndices = MyNew char[bufferSize];
        }

        m_SubmeshList[0]->m_pIndexBuffer->InitializeBuffer( pIndices, bufferSize, MyRE::BufferType_Index, MyRE::BufferUsage_StaticDraw,
                                                            false, 1, bytesPerIndex, category, "Indices" );
    }
}

void MyMesh::CreateClipSpaceQuad(Vector2 minUV, Vector2 maxUV)
{
    unsigned short numVerts = 4;
    unsigned int numIndices = 0;

    // Reinitialize the submesh properties along with the vertex buffer.
    RebuildShapeBuffers( numVerts, VertexFormat_Sprite, MyRE::PrimitiveType_TriangleStrip, 0, MyRE::IndexType_Undefined, "MyMesh_ScreenSpaceQuad" );

    Vertex_Sprite* pVerts = (Vertex_Sprite*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );

    if( pVerts )
    {
        pVerts[0].x = -1; pVerts[0].y =  1;   pVerts[0].u = minUV.x; pVerts[0].v = maxUV.y; // upper left
        pVerts[1].x =  1; pVerts[1].y =  1;   pVerts[1].u = maxUV.x; pVerts[1].v = maxUV.y; // upper right
        pVerts[2].x = -1; pVerts[2].y = -1;   pVerts[2].u = minUV.x; pVerts[2].v = minUV.y; // lower left
        pVerts[3].x =  1; pVerts[3].y = -1;   pVerts[3].u = maxUV.x; pVerts[3].v = minUV.y; // lower right

        m_AABounds.Set( Vector3( 0, 0, 0 ), Vector3( 0.5f, 0.5f, 0 ) );
    }

    m_MeshReady = true;
}

void MyMesh::CreateBox(float boxWidth, float boxHeight, float boxDepth, float startU, float endU, float startV, float endV, unsigned char justificationFlags, Vector3 offset)
{
    unsigned short numVerts = 24;
    unsigned int numIndices = 36;

    // Reinitialize the submesh properties along with the vertex and index buffers.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZUVNorm, MyRE::PrimitiveType_Triangles, numIndices, MyRE::IndexType_U8, "MyMesh_Box" );

    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );
    unsigned char* pIndices = (unsigned char*)m_SubmeshList[0]->m_pIndexBuffer->GetData( true );

    if( pVerts && pIndices )
    {
        float uleft = startU;
        float uright = endU;
        float vtop = startV;
        float vbottom = endV;

        float xleft = 0;
        float xright = boxWidth;
        float ytop = 0;
        float ybottom = -boxHeight;
        float zfront = boxDepth/2;
        float zback = -boxDepth/2;

        if( justificationFlags & Justify_Bottom )
        {
            ytop += boxHeight;
            ybottom += boxHeight;
        }
        else if( justificationFlags & Justify_CenterY )
        {
            ytop += boxHeight / 2.0f;
            ybottom += boxHeight / 2.0f;
        }

        if( justificationFlags & Justify_Right )
        {
            xleft -= boxWidth;
            xright -= boxWidth;
        }
        else if( justificationFlags & Justify_CenterX )
        {
            xleft -= boxWidth / 2.0f;
            xright -= boxWidth / 2.0f;
        }

        xleft += offset.x;
        xright += offset.x;
        ytop += offset.y;
        ybottom += offset.y;
        zfront += offset.z;
        zback += offset.z;

        unsigned char side;

        // Front.
        side = 0;
        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zfront;  pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;    // upper left
        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zfront;  pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;    // upper right
        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zfront;  pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom; // lower left
        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zfront;  pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom; // lower right
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, 0, 1 );

        // Back.
        side = 1;
        pVerts[4*side + 0] = pVerts[1]; pVerts[4*side + 0].pos.z = zback;  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0]; pVerts[4*side + 1].pos.z = zback;  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3]; pVerts[4*side + 2].pos.z = zback;  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2]; pVerts[4*side + 3].pos.z = zback;  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, 0, -1 );

        // Right.
        side = 2;
        pVerts[4*side + 0] = pVerts[1];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 1, 0, 0 );

        // Left.
        side = 3;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( -1, 0, 0 );

        // Bottom.
        side = 4;
        pVerts[4*side + 0] = pVerts[2];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[3];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, -1, 0 );

        // Top.
        side = 5;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[0];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[1];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, 1, 0 );

        for( side=0; side<6; side++ )
        {
            pIndices[6*side + 0] = 4*side + 0;
            pIndices[6*side + 1] = 4*side + 2;
            pIndices[6*side + 2] = 4*side + 1;
            pIndices[6*side + 3] = 4*side + 1;
            pIndices[6*side + 4] = 4*side + 2;
            pIndices[6*side + 5] = 4*side + 3;
        }

        Vector3 center( (xleft + xright) / 2, (ytop + ybottom) / 2, (zfront + zback) / 2 );
        m_AABounds.Set( center, Vector3(boxWidth/2, boxHeight/2, boxDepth/2) );
    }

    m_MeshReady = true;
}

void MyMesh::CreateBox_XYZUV_RGBA(float boxWidth, float boxHeight, float boxDepth, float topStartU, float topEndU, float topStartV, float topEndV, float sideStartU, float sideEndU, float sideStartV, float sideEndV, unsigned char justificationFlags)
{
    unsigned short numVerts = 24;
    unsigned int numIndices = 36;

    // Reinitialize the submesh properties along with the vertex and index buffers.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZUV_RGBA, MyRE::PrimitiveType_Triangles, numIndices, MyRE::IndexType_U8, "MyMesh_BoxXYZUVRGBA" );

    Vertex_XYZUV_RGBA_Alt* pVerts = (Vertex_XYZUV_RGBA_Alt*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );
    unsigned char* pIndices = (unsigned char*)m_SubmeshList[0]->m_pIndexBuffer->GetData( true );

    if( pVerts && pIndices )
    {
        float ulefttop = topStartU;
        float urighttop = topEndU;
        float vtoptop = topStartV;
        float vbottomtop = topEndV;

        float uleftside = sideStartU;
        float urightside = sideEndU;
        float vtopside = sideStartV;
        float vbottomside = sideEndV;

        float xleft = 0;
        float xright = boxWidth;
        float ytop = 0;
        float ybottom = -boxHeight;
        float zfront = boxDepth/2;
        float zback = -boxDepth/2;

        if( justificationFlags & Justify_Bottom )
        {
            ytop += boxHeight;
            ybottom += boxHeight;
        }
        else if( justificationFlags & Justify_CenterY )
        {
            ytop += boxHeight / 2.0f;
            ybottom += boxHeight / 2.0f;
        }

        if( justificationFlags & Justify_Right )
        {
            xleft -= boxWidth;
            xright -= boxWidth;
        }
        else if( justificationFlags & Justify_CenterX )
        {
            xleft -= boxWidth / 2.0f;
            xright -= boxWidth / 2.0f;
        }

        unsigned char side;

        // Front.
        side = 0;
        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zfront;  pVerts[0].uv.x = uleftside;  pVerts[0].uv.y = vtopside;    // upper left
        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zfront;  pVerts[1].uv.x = urightside; pVerts[1].uv.y = vtopside;    // upper right
        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zfront;  pVerts[2].uv.x = uleftside;  pVerts[2].uv.y = vbottomside; // lower left
        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zfront;  pVerts[3].uv.x = urightside; pVerts[3].uv.y = vbottomside; // lower right
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0,1,1 ); //normal.Set( 0, 0, 1 );

        // Back.
        side = 1;
        pVerts[4*side + 0] = pVerts[1]; pVerts[4*side + 0].pos.z = zback;  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0]; pVerts[4*side + 1].pos.z = zback;  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3]; pVerts[4*side + 2].pos.z = zback;  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2]; pVerts[4*side + 3].pos.z = zback;  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0,0.2f,1 ); //normal.Set( 0, 0, -1 );

        // Right.
        side = 2;
        pVerts[4*side + 0] = pVerts[1];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //1,0,0,1 ); //normal.Set( 1, 0, 0 );

        // Left.
        side = 3;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0.2f,0,0,1 ); //normal.Set( -1, 0, 0 );

        // Bottom.
        side = 4;
        pVerts[4*side + 0] = pVerts[2];  pVerts[4*side + 0].uv.x = ulefttop;  pVerts[4*side + 0].uv.y = vtoptop;
        pVerts[4*side + 1] = pVerts[3];  pVerts[4*side + 1].uv.x = urighttop; pVerts[4*side + 1].uv.y = vtoptop;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv.x = ulefttop;  pVerts[4*side + 2].uv.y = vbottomtop;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv.x = urighttop; pVerts[4*side + 3].uv.y = vbottomtop;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0.2f,0,1 ); //normal.Set( 0, -1, 0 );

        // Top.
        side = 5;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[4*4 + 0].uv; // Copy UV's from "bottom".
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[4*4 + 1].uv;
        pVerts[4*side + 2] = pVerts[0];  pVerts[4*side + 2].uv = pVerts[4*4 + 2].uv;
        pVerts[4*side + 3] = pVerts[1];  pVerts[4*side + 3].uv = pVerts[4*4 + 3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,1,0,1 ); //normal.Set( 0, 1, 0 );

        for( side=0; side<6; side++ )
        {
            pIndices[6*side + 0] = 4*side + 0;
            pIndices[6*side + 1] = 4*side + 2;
            pIndices[6*side + 2] = 4*side + 1;
            pIndices[6*side + 3] = 4*side + 1;
            pIndices[6*side + 4] = 4*side + 2;
            pIndices[6*side + 5] = 4*side + 3;
        }

        Vector3 center( (xleft + xright) / 2, (ytop + ybottom) / 2, (zfront + zback) / 2 );
        m_AABounds.Set( center, Vector3(boxWidth/2, boxHeight/2, boxDepth/2) );
    }

    m_MeshReady = true;
}

void MyMesh::SetBoxVertexColors(ColorByte TL, ColorByte TR, ColorByte BL, ColorByte BR)
{
    MyAssert( m_SubmeshList.Count() == 1 );

    Vertex_XYZUV_RGBA_Alt* pVerts = (Vertex_XYZUV_RGBA_Alt*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );

    int side;

    // Front.
    side = 0;
    pVerts[0].col = BL;
    pVerts[1].col = BR;
    pVerts[2].col = BL;
    pVerts[3].col = BR;

    // Back.
    side = 1;
    pVerts[4*side + 0].col = TR;
    pVerts[4*side + 1].col = TL;
    pVerts[4*side + 2].col = TR;
    pVerts[4*side + 3].col = TL;

    // Right.
    side = 2;
    pVerts[4*side + 0].col = BR;
    pVerts[4*side + 1].col = TR;
    pVerts[4*side + 2].col = BR;
    pVerts[4*side + 3].col = TR;

    // Left.
    side = 3;
    pVerts[4*side + 0].col = TL;
    pVerts[4*side + 1].col = BL;
    pVerts[4*side + 2].col = TL;
    pVerts[4*side + 3].col = BL;

    // Bottom.
    side = 4;
    pVerts[4*side + 0].col = BL;
    pVerts[4*side + 1].col = BR;
    pVerts[4*side + 2].col = TL;
    pVerts[4*side + 3].col = TR;

    // Top.
    side = 5;
    pVerts[4*side + 0].col = TL;
    pVerts[4*side + 1].col = TR;
    pVerts[4*side + 2].col = BL;
    pVerts[4*side + 3].col = BR;

    m_MeshReady = true;
}

void MyMesh::CreateCylinder(float radius, unsigned short numSegments, float edgeRadius, float height, float topStartU, float topEndU, float topStartV, float topEndV, float sideStartU, float sideEndU, float sideStartV, float sideEndV)
{
    float uperc, vperc;

    unsigned short topverts = numSegments + 1;
    unsigned short edgeverts = numSegments * 2;
    unsigned short sideverts = (numSegments + 1) * 2; // +1 since I'm doubling a column of verts to allow mapping from atlas.
    unsigned int numtris = numSegments * 8;

    unsigned short numVerts = (topverts + edgeverts)*2 + sideverts;
    unsigned int numIndices = numtris*3;

    // Reinitialize the submesh properties along with the vertex and index buffers.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZUVNorm, MyRE::PrimitiveType_Triangles, numIndices, MyRE::IndexType_U16, "MyMesh_Cylinder" );

    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );
    unsigned short* pIndices = (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->GetData( true );

    GLushort vertnum = 0;

    float topsizeu = topEndU - topStartU;
    float topsizev = topEndV - topStartV;

    // Create top center vert.
    pVerts[vertnum].pos.Set( 0, height, 0 );
    pVerts[vertnum].uv.Set( topStartU + (topsizeu / 2.0f), topStartV + (topsizev / 2.0f) );
    pVerts[vertnum].normal.Set( 0, 1, 0 );
    vertnum++;

    float uvclamplow = 0.01f;
    float uvclamphigh = 1 - uvclamplow;// 0.98f;

    //               TOP                      BOTTOM - 17+
    //z=1        /----6----\                /----6----\
    //          /           \              /           \
    //         8   /--5--\   4            8   /--5--\   4
    //        /   7       3   \          /   7       3   \
    //       /   /         \   \        /   /         \   \
    //z=0   10  9     0     1   2      10  9     0     1   2
    //       \   \         /   /        \   \         /   /
    //        \   11      15  /          \   11      15  /
    //         12  \--13-/   16           12  \--13-/   16
    //          \           /              \           /
    //z=-1       \----14---/                \----14---/

    int flipz = 1;
#if MYFW_RIGHTHANDED
    flipz = -1;
#endif

    // Create top inner and outer(edge) circle verts.
    for( int i=0; i<numSegments; i++ )
    {
        float radianspersegment = PI*2/numSegments;
        float radians = radianspersegment * i;

        // inner vert
        pVerts[vertnum].pos.x = cosf( radians ) * (radius - edgeRadius);
        pVerts[vertnum].pos.z = sinf( radians ) * (radius - edgeRadius) * flipz;
        pVerts[vertnum].pos.y = height;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = 1 - (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topStartU + topsizeu * uperc, topStartV + topsizev * vperc );
        pVerts[vertnum].normal.Set( 0, 1, 0 );
        vertnum++;

        // Outer vert.
        pVerts[vertnum].pos.x = cosf( radians ) * (radius);
        pVerts[vertnum].pos.z = sinf( radians ) * (radius) * flipz;
        pVerts[vertnum].pos.y = height;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = 1 - (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topStartU + topsizeu * uperc, topStartV + topsizev * vperc );
        pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 1, pVerts[vertnum].pos.z );
        pVerts[vertnum].normal.Normalize();
        vertnum++;
    }

    // Create bottom center vert.
    pVerts[vertnum].pos.Set( 0, 0, 0 );
    pVerts[vertnum].uv.Set( topStartU + (topsizeu / 2.0f), topStartV + (topsizev / 2.0f) );
    pVerts[vertnum].normal.Set( 0, -1, 0 );
    vertnum++;

    // Create bottom inner and outer(edge) circle verts.
    for( int i=0; i<numSegments; i++ )
    {
        float radianspersegment = PI*2/numSegments;
        float radians = radianspersegment * i;

        // Inner vert.
        pVerts[vertnum].pos.x = cosf( radians ) * (radius - edgeRadius);
        pVerts[vertnum].pos.z = sinf( radians ) * (radius - edgeRadius) * flipz;
        pVerts[vertnum].pos.y = 0;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = 1 - (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topEndU - topsizeu * uperc, topStartV + topsizev * vperc );
        pVerts[vertnum].normal.Set( 0, -1, 0 );
        vertnum++;

        // Outer vert.
        pVerts[vertnum].pos.x = cosf( radians ) * (radius);
        pVerts[vertnum].pos.z = sinf( radians ) * (radius) * flipz;
        pVerts[vertnum].pos.y = 0;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = 1 - (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topEndU - topsizeu * uperc, topStartV + topsizev * vperc );
        pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, -1, pVerts[vertnum].pos.z );
        pVerts[vertnum].normal.Normalize();
        vertnum++;
    }

    // Create side verts.
    GLushort firstsidevert = vertnum;
    {
        int topbase = 0;
        int bottombase = numSegments * 2 + 1;
        for( int i=0; i<numSegments+1; i++ )
        {
            int oldvertindex = i;
            if( i == numSegments )
                oldvertindex = 0;

            // Top vert.
            pVerts[vertnum].pos.x = pVerts[topbase + oldvertindex*2 + 2].pos.x;
            pVerts[vertnum].pos.z = pVerts[topbase + oldvertindex*2 + 2].pos.z;
            pVerts[vertnum].pos.y = height;
            pVerts[vertnum].uv.x = sideStartU + (sideEndU - sideStartU)*i / numSegments;
            pVerts[vertnum].uv.y = sideStartV;
            pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 0, pVerts[vertnum].pos.z );
            pVerts[vertnum].normal.Normalize();
            vertnum++;

            // Bottom vert.
            pVerts[vertnum].pos.x = pVerts[bottombase + oldvertindex*2 + 2].pos.x;
            pVerts[vertnum].pos.z = pVerts[bottombase + oldvertindex*2 + 2].pos.z;
            pVerts[vertnum].pos.y = 0;
            pVerts[vertnum].uv.x = sideStartU + (sideEndU - sideStartU)*i / numSegments;
            pVerts[vertnum].uv.y = sideEndV;
            pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 0, pVerts[vertnum].pos.z );
            pVerts[vertnum].normal.Normalize();
            vertnum++;
        }
    }

    int indexnum = 0;

    // Setup top center triangles.
    if( true )
    {
        GLushort baseindex = 0;
        for( GLushort i=0; i<numSegments; i++ )
        {
            // 0,3,1,   0,5,3,   0,7,5...
            pIndices[indexnum + 0] = 0;
            pIndices[indexnum + 2] = baseindex + i*2 + 1;
            if( i != numSegments-1 )
                pIndices[indexnum + 1] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 1] = baseindex + 1;

            indexnum += 3;
        }
    }

    // Setup top outer edge triangles.
    if( true )
    {
        for( GLushort i=0; i<numSegments; i++ )
        {
            // 1,3,2,   2,3,4,   3,5,4...
            pIndices[indexnum + 0] = i*2 + 1;
            pIndices[indexnum + 2] = i*2 + 2;
            if( i != numSegments-1 )
                pIndices[indexnum + 1] = (i+1)*2 + 1;
            else
                pIndices[indexnum + 1] = 0*2 + 1;
            indexnum += 3;

            pIndices[indexnum + 0] = (i)*2 + 2;
            if( i != numSegments-1 )
            {
                pIndices[indexnum + 2] = (i+1)*2 + 2;
                pIndices[indexnum + 1] = (i+1)*2 + 1;
            }
            else
            {
                pIndices[indexnum + 2] = (0)*2 + 2;
                pIndices[indexnum + 1] = (0)*2 + 1;
            }
            indexnum += 3;
        }
    }

    // Setup side triangles.
    if( true )
    {
        for( GLushort i=0; i<numSegments; i++ )
        {
            pIndices[indexnum + 0] = firstsidevert + i*2;
            pIndices[indexnum + 2] = firstsidevert + i*2 + 1;
            pIndices[indexnum + 1] = firstsidevert + i*2 + 2;
            indexnum += 3;

            pIndices[indexnum + 0] = firstsidevert + i*2 + 1;
            pIndices[indexnum + 2] = firstsidevert + i*2 + 3;
            pIndices[indexnum + 1] = firstsidevert + i*2 + 2;
            indexnum += 3;
        }
    }

    // Setup bottom center triangles.
    if( true )
    {
        GLushort baseindex = (GLushort)(numSegments*2 + 1);
        for( GLushort i=0; i<numSegments; i++ )
        {
            pIndices[indexnum + 0] = baseindex + 0;
            pIndices[indexnum + 1] = baseindex + i*2 + 1;
            if( i != numSegments-1 )
                pIndices[indexnum + 2] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 2] = baseindex + 1;

            indexnum += 3;
        }
    }

    // Setup bottom outer edge triangles.
    if( true )
    {
        GLushort baseindex = (GLushort)(numSegments*2 + 1);
        for( GLushort i=0; i<numSegments; i++ )
        {
            pIndices[indexnum + 0] = baseindex + i*2 + 1;
            pIndices[indexnum + 1] = baseindex + i*2 + 2;
            if( i != numSegments-1 )
                pIndices[indexnum + 2] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 2] = baseindex + 0*2 + 1;
            indexnum += 3;

            pIndices[indexnum + 0] = baseindex + (i)*2 + 2;
            if( i != numSegments-1 )
            {
                pIndices[indexnum + 1] = baseindex + (i+1)*2 + 2;
                pIndices[indexnum + 2] = baseindex + (i+1)*2 + 1;
            }
            else
            {
                pIndices[indexnum + 1] = baseindex + (0)*2 + 2;
                pIndices[indexnum + 2] = baseindex + (0)*2 + 1;
            }
            indexnum += 3;
        }
    }

    Vector3 center( 0, height/2, 0 );
    m_AABounds.Set( center, Vector3(radius, height/2, radius) );

    m_MeshReady = true;
}

void MyMesh::CreatePlane(Vector3 topLeftPos, Vector2 size, Vector2Int vertCount, Vector2 uvStart, Vector2 uvRange, bool createTriangles)
{
    //LOGInfo( LOGTag, "MyMesh::CreatePlane\n" );

    if( vertCount.x <= 0 || vertCount.y <= 0 || vertCount.x * vertCount.y > 65535 )
        return;

    unsigned int numTris = (vertCount.x - 1) * (vertCount.y - 1) * 2;
    unsigned int numVerts = vertCount.x * vertCount.y;
    unsigned int numIndices = numTris * 3;

    if( createTriangles == false )
        numIndices = numVerts;

    // Reinitialize the submesh properties along with the vertex and index buffers.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZUV, MyRE::PrimitiveType_Triangles, numIndices, MyRE::IndexType_U16, "MyMesh_Plane" );

    Vertex_XYZUV* pVerts = (Vertex_XYZUV*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );
    unsigned short* pIndices = (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->GetData( true );

    for( int y = 0; y < vertCount.y; y++ )
    {
        for( int x = 0; x < vertCount.x; x++ )
        {
            unsigned short index = (unsigned short)(y * vertCount.x + x);

            pVerts[index].x = topLeftPos.x + size.x / (vertCount.x - 1) * x;
            pVerts[index].y = topLeftPos.y;
            pVerts[index].z = topLeftPos.z - size.y / (vertCount.y - 1) * y;

            pVerts[index].u = uvStart.x + x * uvRange.x / (vertCount.x - 1);
            pVerts[index].v = uvStart.y + y * uvRange.y / (vertCount.y - 1);

            if( createTriangles == false )
                pIndices[index] = index;
        }
    }

    if( createTriangles )
    {
        for( int y = 0; y < vertCount.y - 1; y++ )
        {
            for( int x = 0; x < vertCount.x - 1; x++ )
            {
                int elementIndex = (y * (vertCount.x-1) + x) * 6;
                unsigned short vertexIndex = (unsigned short)(y * vertCount.x + x);

                pIndices[ elementIndex + 0 ] = vertexIndex + 0;
                pIndices[ elementIndex + 1 ] = vertexIndex + 1;
                pIndices[ elementIndex + 2 ] = vertexIndex + (unsigned short)vertCount.x;

                pIndices[ elementIndex + 3 ] = vertexIndex + 1;
                pIndices[ elementIndex + 4 ] = vertexIndex + (unsigned short)vertCount.x + 1;
                pIndices[ elementIndex + 5 ] = vertexIndex + (unsigned short)vertCount.x;
            }
        }
    }

    Vector3 center( topLeftPos.x + size.x/2, topLeftPos.y, topLeftPos.z + size.y/ 2 );
    m_AABounds.Set( center, Vector3(size.x/2, 0, size.y/2) );

    m_MeshReady = true;
};

//
//   0-----1/4-----5/8-----9    <-- vertcount.x * 2 - 2
//   |      |       |      |
//   |      |       |      |
//   2-----3/6-----7/10----11   <-- vertcount.x * 2 - 2
//  12----13/16---17/20----22   <-- vertcount.x * 2 - 2
//   |      |       |      |
//   |      |       |      |
//  14----15/18---19/21----23   <-- vertcount.x * 2 - 2
//
void MyMesh::CreatePlaneUVsNotShared(Vector3 topLeftPos, Vector2 size, Vector2Int vertCount, Vector2 uvStart, Vector2 uvRange, bool createTriangles)
{
    //LOGInfo( LOGTag, "MyMesh::CreatePlaneUVsNotShared\n" );

    MyAssert( vertCount.x > 0 && vertCount.y > 0 );
    if( (vertCount.x - 1) * (vertCount.y - 1) * 4 > 65535 )
    {
        LOGInfo( LOGTag, "MyMesh::CreatePlaneUVsNotShared - too many verts needed for unsigned short indices.\n" );
        return;
    }

    unsigned int numQuads = (vertCount.x - 1) * (vertCount.y - 1);
    unsigned int numTris = numQuads * 2;
    unsigned int numVerts = numQuads * 4;
    unsigned int numIndices = numTris * 3;

    //if( createtriangles == false )
    //    numindices = numverts;

    // Reinitialize the submesh properties along with the vertex and index buffers.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZUV, MyRE::PrimitiveType_Triangles, numIndices, MyRE::IndexType_U16, "MyMesh_Plane" );

    Vertex_XYZUV* pVerts = (Vertex_XYZUV*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );
    unsigned short* pIndices = (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->GetData( true );

    // Loop through the quads.
    unsigned short vertex = 0;
    int index = 0;
    for( int y = 0; y < vertCount.y-1; y++ )
    {
        for( int x = 0; x < vertCount.x-1; x++ )
        {
            // 4 verts and 6 indices per quad
            pVerts[vertex+0].x = topLeftPos.x + size.x / (vertCount.x - 1) * x;
            pVerts[vertex+0].y = topLeftPos.y;
            pVerts[vertex+0].z = topLeftPos.z - size.y / (vertCount.y - 1) * y;
            pVerts[vertex+0].u = uvStart.x;
            pVerts[vertex+0].v = uvStart.y;

            pVerts[vertex+1].x = topLeftPos.x + size.x / (vertCount.x - 1) * (x+1);
            pVerts[vertex+1].y = topLeftPos.y;
            pVerts[vertex+1].z = topLeftPos.z - size.y / (vertCount.y - 1) * y;
            pVerts[vertex+1].u = uvStart.x + uvRange.x;
            pVerts[vertex+1].v = uvStart.y;

            pVerts[vertex+2].x = topLeftPos.x + size.x / (vertCount.x - 1) * x;
            pVerts[vertex+2].y = topLeftPos.y;
            pVerts[vertex+2].z = topLeftPos.z - size.y / (vertCount.y - 1) * (y+1);
            pVerts[vertex+2].u = uvStart.x;
            pVerts[vertex+2].v = uvStart.y + uvRange.y;

            pVerts[vertex+3].x = topLeftPos.x + size.x / (vertCount.x - 1) * (x+1);
            pVerts[vertex+3].y = topLeftPos.y;
            pVerts[vertex+3].z = topLeftPos.z - size.y / (vertCount.y - 1) * (y+1);
            pVerts[vertex+3].u = uvStart.x + uvRange.x;
            pVerts[vertex+3].v = uvStart.y + uvRange.y;

            pIndices[index+0] = vertex+0;
            pIndices[index+1] = vertex+1;
            pIndices[index+2] = vertex+2;
            pIndices[index+3] = vertex+1;
            pIndices[index+4] = vertex+3;
            pIndices[index+5] = vertex+2;

            vertex += 4;
            index += 6;

            //if( createtriangles == false )
            //    pIndices[index] = index;
        }
    }

    Vector3 center( topLeftPos.x + size.x/2, topLeftPos.y, topLeftPos.z + size.y/ 2 );
    m_AABounds.Set( center, Vector3(size.x/2, 0, size.y/2) );

    m_MeshReady = true;
};

void MyMesh::CreateIcosphere(float radius, unsigned int recursionLevel)
{
    // From http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html

    unsigned int numtris = 20;

    int numVerts = 12;
    unsigned int numIndices = numtris * 3;

    // Reinitialize the submesh properties along with the vertex and index buffers.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZUVNorm, MyRE::PrimitiveType_Triangles, numIndices, MyRE::IndexType_U8, "MyMesh_Icosphere" );

    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );
    unsigned char* pIndices = (unsigned char*)m_SubmeshList[0]->m_pIndexBuffer->GetData( true );

    //m_SubmeshList[0]->m_pIndexBuffer->m_BytesPerIndex = bytesperindex;

    // Create 12 vertices of a icosahedron.
    float t = (1.0f + sqrtf(5.0f)) / 2.0f;

    pVerts[ 0].pos.Set( -1,  t,  0 );
    pVerts[ 1].pos.Set(  1,  t,  0 );
    pVerts[ 2].pos.Set( -1, -t,  0 );
    pVerts[ 3].pos.Set(  1, -t,  0 );

    pVerts[ 4].pos.Set(  0, -1,  t );
    pVerts[ 5].pos.Set(  0,  1,  t );
    pVerts[ 6].pos.Set(  0, -1, -t );
    pVerts[ 7].pos.Set(  0,  1, -t );

    pVerts[ 8].pos.Set(  t,  0, -1 );
    pVerts[ 9].pos.Set(  t,  0,  1 );
    pVerts[10].pos.Set( -t,  0, -1 );
    pVerts[11].pos.Set( -t,  0,  1 );

    for( int i=0; i<12; i++ )
    {
        pVerts[i].pos *= radius;
        pVerts[i].normal = pVerts[i].pos.GetNormalized();
    }

    // Create 20 triangles of the icosahedron.
    unsigned char indexlist[] =
    {
       0, 11,  5,    0,  5,  1,    0,  1,  7,    0,  7, 10,    0, 10, 11,  // 5 faces around point 0.
       1,  5,  9,    5, 11,  4,   11, 10,  2,   10,  7,  6,    7,  1,  8,  // 5 adjacent faces.
       3,  9,  4,    3,  4,  2,    3,  2,  6,    3,  6,  8,    3,  8,  9,  // 5 faces around point 3.
       4,  9,  5,    2,  4, 11,    6,  2, 10,    8,  6,  7,    9,  8,  1,  // 5 adjacent faces.
    };

    for( int i=0; i<60; i++ )
        pIndices[i] = indexlist[i];

    m_AABounds.Set( Vector3(0), Vector3(radius) );

    m_MeshReady = true;

    //public class IcoSphereCreator
    //{
    //    private struct TriangleIndices
    //    {
    //        public int v1;
    //        public int v2;
    //        public int v3;

    //        public TriangleIndices(int v1, int v2, int v3)
    //        {
    //            this.v1 = v1;
    //            this.v2 = v2;
    //            this.v3 = v3;
    //        }
    //    }

    //    private MeshGeometry3D geometry;
    //    private int index;
    //    private Dictionary<Int64, int> middlePointIndexCache;

    //    // add vertex to mesh, fix position to be on unit sphere, return index
    //    private int addVertex(Point3D p)
    //    {
    //        double length = Math.Sqrt(p.X * p.X + p.Y * p.Y + p.Z * p.Z);
    //        geometry.Positions.Add(new Point3D(p.X/length, p.Y/length, p.Z/length));
    //        return index++;
    //    }

    //    // return index of point in the middle of p1 and p2
    //    private int getMiddlePoint(int p1, int p2)
    //    {
    //        // first check if we have it already
    //        bool firstIsSmaller = p1 < p2;
    //        Int64 smallerIndex = firstIsSmaller ? p1 : p2;
    //        Int64 greaterIndex = firstIsSmaller ? p2 : p1;
    //        Int64 key = (smallerIndex << 32) + greaterIndex;

    //        int ret;
    //        if (this.middlePointIndexCache.TryGetValue(key, out ret))
    //        {
    //            return ret;
    //        }

    //        // not in cache, calculate it
    //        Point3D point1 = this.geometry.Positions[p1];
    //        Point3D point2 = this.geometry.Positions[p2];
    //        Point3D middle = new Point3D(
    //            (point1.X + point2.X) / 2.0, 
    //            (point1.Y + point2.Y) / 2.0, 
    //            (point1.Z + point2.Z) / 2.0);

    //        // add vertex makes sure point is on unit sphere
    //        int i = addVertex(middle); 

    //        // store it, return index
    //        this.middlePointIndexCache.Add(key, i);
    //        return i;
    //    }

    //    public MeshGeometry3D Create(int recursionLevel)
    //    {
    //        this.geometry = new MeshGeometry3D();
    //        this.middlePointIndexCache = new Dictionary<long, int>();
    //        this.index = 0;

    //        // create 12 vertices of a icosahedron
    //        var t = (1.0 + Math.Sqrt(5.0)) / 2.0;

    //        addVertex(new Point3D(-1,  t,  0));
    //        addVertex(new Point3D( 1,  t,  0));
    //        addVertex(new Point3D(-1, -t,  0));
    //        addVertex(new Point3D( 1, -t,  0));

    //        addVertex(new Point3D( 0, -1,  t));
    //        addVertex(new Point3D( 0,  1,  t));
    //        addVertex(new Point3D( 0, -1, -t));
    //        addVertex(new Point3D( 0,  1, -t));

    //        addVertex(new Point3D( t,  0, -1));
    //        addVertex(new Point3D( t,  0,  1));
    //        addVertex(new Point3D(-t,  0, -1));
    //        addVertex(new Point3D(-t,  0,  1));


    //        // create 20 triangles of the icosahedron
    //        var faces = new List<TriangleIndices>();

    //        // 5 faces around point 0
    //        faces.Add(new TriangleIndices(0, 11, 5));
    //        faces.Add(new TriangleIndices(0, 5, 1));
    //        faces.Add(new TriangleIndices(0, 1, 7));
    //        faces.Add(new TriangleIndices(0, 7, 10));
    //        faces.Add(new TriangleIndices(0, 10, 11));

    //        // 5 adjacent faces 
    //        faces.Add(new TriangleIndices(1, 5, 9));
    //        faces.Add(new TriangleIndices(5, 11, 4));
    //        faces.Add(new TriangleIndices(11, 10, 2));
    //        faces.Add(new TriangleIndices(10, 7, 6));
    //        faces.Add(new TriangleIndices(7, 1, 8));

    //        // 5 faces around point 3
    //        faces.Add(new TriangleIndices(3, 9, 4));
    //        faces.Add(new TriangleIndices(3, 4, 2));
    //        faces.Add(new TriangleIndices(3, 2, 6));
    //        faces.Add(new TriangleIndices(3, 6, 8));
    //        faces.Add(new TriangleIndices(3, 8, 9));

    //        // 5 adjacent faces 
    //        faces.Add(new TriangleIndices(4, 9, 5));
    //        faces.Add(new TriangleIndices(2, 4, 11));
    //        faces.Add(new TriangleIndices(6, 2, 10));
    //        faces.Add(new TriangleIndices(8, 6, 7));
    //        faces.Add(new TriangleIndices(9, 8, 1));


    //        // refine triangles
    //        for (int i = 0; i < recursionLevel; i++)
    //        {
    //            var faces2 = new List<TriangleIndices>();
    //            foreach (var tri in faces)
    //            {
    //                // replace triangle by 4 triangles
    //                int a = getMiddlePoint(tri.v1, tri.v2);
    //                int b = getMiddlePoint(tri.v2, tri.v3);
    //                int c = getMiddlePoint(tri.v3, tri.v1);

    //                faces2.Add(new TriangleIndices(tri.v1, a, c));
    //                faces2.Add(new TriangleIndices(tri.v2, b, a));
    //                faces2.Add(new TriangleIndices(tri.v3, c, b));
    //                faces2.Add(new TriangleIndices(a, b, c));
    //            }
    //            faces = faces2;
    //        }

    //        // done, now add triangles to mesh
    //        foreach (var tri in faces)
    //        {
    //            this.geometry.TriangleIndices.Add(tri.v1);
    //            this.geometry.TriangleIndices.Add(tri.v2);
    //            this.geometry.TriangleIndices.Add(tri.v3);
    //        }

    //        return this.geometry;        
    //    }
    //}
}

void MyMesh::CreateCone(float radius, unsigned short numSegments, float height)
{
    int numVerts = 1 + numSegments + 1; // Center + loop + repeat of first to close shape.

    // Reinitialize the submesh properties along with the vertex buffer.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZ, MyRE::PrimitiveType_TriangleFan, 0, MyRE::IndexType_Undefined, "MyMesh_Cone" );

    Vertex_XYZ* pVerts = (Vertex_XYZ*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );

    float angleChange = -2.0f * PI / numSegments;

    // Center.
    pVerts[0].x = 0.0f;
    pVerts[0].y = 0.0f;
    pVerts[0].z = 0.0f;

    // Loop.
    for( int i=0; i<numSegments; i++ )
    {
        pVerts[1 + i].x = cosf( i*angleChange ) * radius;
        pVerts[1 + i].y = -height;
        pVerts[1 + i].z = sinf( i*angleChange ) * radius;
    }

    // Repeat vert 1 to close shape.
    pVerts[numVerts-1] = pVerts[1];

    m_AABounds.Set( Vector3(0, height/2, 0), Vector3(radius, height/2, radius) );

    m_MeshReady = true;
}

void MyMesh::Create2DCircle(float radius, unsigned int numberOfSegments)
{
    int numVerts = numberOfSegments;

    // Reinitialize the submesh properties along with the vertex buffer.
    RebuildShapeBuffers( numVerts, VertexFormat_Sprite, MyRE::PrimitiveType_TriangleFan, 0, MyRE::IndexType_Undefined, "MyMesh_2DCircle" );

    Vertex_Sprite* pVerts = (Vertex_Sprite*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );

    float anglechange = -2.0f * PI / numVerts;

    for( int i=0; i<numVerts; i++ )
    {
        pVerts[i].x = cosf( i*anglechange ) * radius;
        pVerts[i].y = sinf( i*anglechange ) * radius;

        pVerts[i].u = cosf( i*anglechange );
        pVerts[i].v = sinf( i*anglechange );
    }

    m_AABounds.Set( Vector3(0), Vector3(radius, radius, 0) );

    m_MeshReady = true;
}

void MyMesh::Create2DArc(Vector3 origin, float startAngle, float endAngle, float startRadius, float endRadius, unsigned int numberOfSegments)
{
    int numVerts = numberOfSegments * 2 + 2;

    // Reinitialize the submesh properties along with the vertex buffer.
    RebuildShapeBuffers( numVerts, VertexFormat_Sprite, MyRE::PrimitiveType_TriangleStrip, 0, MyRE::IndexType_Undefined, "MyMesh_2DArc" );

    Vertex_Sprite* pVerts = (Vertex_Sprite*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );

    float percentofcircle = (startAngle - endAngle)/360.0f;
    float anglechange = (PI*2 * percentofcircle) / numberOfSegments * -1;

    float startrad = startAngle/180.0f * PI;

    // Normals for 2D shapes are set to (0,1,0), so plot them out on the X/Z plane.
    for( unsigned int i=0; i<numberOfSegments + 1; i++ )
    {
        pVerts[i*2 + 0].x = cosf( startrad + i*anglechange ) * endRadius;
        pVerts[i*2 + 0].y = sinf( startrad + i*anglechange ) * endRadius;
        pVerts[i*2 + 0].u = cosf( startrad + i*anglechange );
        pVerts[i*2 + 0].v = sinf( startrad + i*anglechange );

        pVerts[i*2 + 1].x = cosf( startrad + i*anglechange ) * startRadius;
        pVerts[i*2 + 1].y = sinf( startrad + i*anglechange ) * startRadius;
        pVerts[i*2 + 1].u = cosf( startrad + i*anglechange );
        pVerts[i*2 + 1].v = sinf( startrad + i*anglechange );
    }

    m_AABounds.Set( Vector3(0), Vector3(endRadius, endRadius, 0) );

    m_MeshReady = true;
}

void SetVertex_XYZUV_RGBA(Vertex_XYZUV_RGBA* pVert, float x, float y, float z, float u, float v, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    pVert->x = x;
    pVert->y = y;
    pVert->z = z;
    pVert->u = u;
    pVert->v = v;
    pVert->r = r;
    pVert->g = g;
    pVert->b = b;
    pVert->a = a;
}

void MyMesh::CreateGrass(Vector3 topLeftPos, Vector2 size, Vector2Int bladeCount, Vector2 bladeSize)
{
    LOGInfo( LOGTag, "MyMesh::CreateGrass numverts: %d,%d\n", bladeCount.x, bladeCount.y );

    int numVerts = bladeCount.x * bladeCount.y * 3;
    unsigned int numIndices = 0;

    // Reinitialize the submesh properties along with the vertex buffer.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZUV_RGBA, MyRE::PrimitiveType_Triangles, 0, MyRE::IndexType_Undefined, "MyMesh_Grass" );

    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );

    for( int y = 0; y < bladeCount.y; y++ )
    {
        for( int x = 0; x < bladeCount.x; x++ )
        {
            unsigned int index = ((y * bladeCount.x + x) * 3);

            float posx = topLeftPos.x + size.x / (bladeCount.x - 1) * x + (rand()%200/100.0f - 1.0f) * bladeSize.x/2;
            float posz = topLeftPos.z + size.y / (bladeCount.y - 1) * y + (rand()%200/100.0f - 1.0f) * bladeSize.x/2;

            SetVertex_XYZUV_RGBA( &pVerts[index+0],
                                    posx, topLeftPos.y, posz,
                                    180 * PI/180.0f, bladeSize.x,
                                    40, 192 + rand()%24, 40, 255 );

            SetVertex_XYZUV_RGBA( &pVerts[index+1],
                                    posx, topLeftPos.y + bladeSize.y, posz,
                                    0 * PI/180.0f, 0,
                                    40, pVerts[index+0].g - 64, 40, 255 );

            SetVertex_XYZUV_RGBA( &pVerts[index+2],
                                    posx, topLeftPos.y, posz,
                                    0 * PI/180.0f, bladeSize.x,
                                    40, pVerts[index+0].g, 40, 255 );
        }
    }

    Vector3 center( topLeftPos.x + size.x/2, topLeftPos.y + bladeSize.y/2, topLeftPos.z + size.y/ 2 );
    m_AABounds.Set( center, Vector3(size.x/2, bladeSize.y/2, size.y/2) );

    m_MeshReady = true;
};

void MyMesh::CreateEditorLineGridXZ(Vector3 center, float spacing, int halfNumBars)
{
    unsigned char numVerts = (unsigned char)((halfNumBars*2+1) * 2 * 2);
    unsigned int numIndices = (halfNumBars*2+1) * 2 * 2; // halfNumBars*2+1centerline * 2axis * 2indicesperline.

    // Reinitialize the submesh properties along with the vertex and index buffers.
    RebuildShapeBuffers( numVerts, VertexFormat_XYZ, MyRE::PrimitiveType_Lines, numIndices, MyRE::IndexType_U8, "MyMesh_GridPlane" );

    Vertex_XYZ* pVerts = (Vertex_XYZ*)m_SubmeshList[0]->m_pVertexBuffer->GetData( true );
    unsigned char* pIndices = (unsigned char*)m_SubmeshList[0]->m_pIndexBuffer->GetData( true );

    unsigned char vertnum = 0;
    int indexnum = 0;

    // Create verts.
    for( int i = -halfNumBars; i<=halfNumBars; i++ )
    {
        pVerts[vertnum].x = center.x + i*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + halfNumBars*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + i*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + -halfNumBars*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + halfNumBars*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + i*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + -halfNumBars*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + i*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;
    }

    m_AABounds.Set( center, Vector3(halfNumBars*spacing, 0, halfNumBars*spacing) );

    m_MeshReady = true;
}

void MyMesh::CreateEditorTransformGizmoAxis(float length, float thickness)
{
    CreateCylinder( thickness, 4, 0, length, 0, 1, 0, 1, 0, 1, 0, 1 );
}
