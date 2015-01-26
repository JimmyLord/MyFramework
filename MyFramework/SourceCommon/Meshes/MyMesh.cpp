//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MyMesh.h"

MyMesh::MyMesh()
{
    m_pSourceFile = 0;
    m_MeshReady = false;

    m_VertexFormat = -1;

    m_pVertexBuffer = 0;
    m_pIndexBuffer = 0;

    m_pShaderGroup = 0;

    m_NumVertsToDraw = 0;
    m_NumIndicesToDraw = 0;
    m_PrimitiveType = GL_TRIANGLES;

    m_pTexture = 0;
    m_Tint = ColorByte(255,255,255,255);
    m_SpecColor = ColorByte(255,255,255,255);
    m_Shininess = 200;

    m_Position.SetIdentity();

    g_pMeshManager->AddMesh( this );
}

MyMesh::~MyMesh()
{
    if( this->Prev )
        this->Remove();

    SAFE_RELEASE( m_pSourceFile );

    SAFE_RELEASE( m_pVertexBuffer );
    SAFE_RELEASE( m_pIndexBuffer );
}

void MyMesh::CreateBuffers(int vertexformat, unsigned short numverts, unsigned int numindices, bool dynamic)
{
    assert( m_pVertexBuffer == 0 );
    assert( m_pIndexBuffer == 0 );

    GLenum usage;
    int numbuffers;
    if( dynamic )
    {
        usage = GL_DYNAMIC_DRAW;
        //numbuffers = 1;
        numbuffers = 2;
    }
    else
    {
        usage = GL_STATIC_DRAW;
        numbuffers = 1;
    }

    if( m_pVertexBuffer == 0 )
    {
        //m_NumVerts = numverts;
        m_NumVertsToDraw = numverts;
        m_VertexFormat = vertexformat;

        if( m_VertexFormat == VertexFormat_XYZUV_RGBA )
        {
            Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[numverts];
            m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUV_RGBA)*numverts, GL_ARRAY_BUFFER, usage, false, numbuffers, VertexFormat_XYZUV_RGBA, "MyMesh", "Verts" );
        }
    }

    if( m_pIndexBuffer == 0 )
    {
        //m_NumIndices = numindices;
        m_NumIndicesToDraw = numindices;

        unsigned short* pIndices = MyNew unsigned short[numindices];
        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned short)*numindices, GL_ELEMENT_ARRAY_BUFFER, usage, false, numbuffers, 2, "MyMesh", "Verts" );
    }

    //m_pVAO = g_pBufferManager->CreateVAO();
}

void MyMesh::CreateFromOBJFile(MyFileObject* pFile)
{
    pFile->AddRef();
    SAFE_RELEASE( m_pSourceFile );
    m_pSourceFile = pFile;

    m_MeshReady = false;

    if( pFile->m_FileReady )
    {
        LoadBasicOBJ( pFile->m_pBuffer, &m_pVertexBuffer, &m_pIndexBuffer, false );

        m_VertexFormat = m_pVertexBuffer->m_VertexFormat;
        m_NumIndicesToDraw = m_pIndexBuffer->m_DataSize / m_pIndexBuffer->m_BytesPerIndex;

        m_MeshReady = true;
    }
}

void MyMesh::CreateBox(float boxw, float boxh, float boxd, float startu, float endu, float startv, float endv, unsigned char justificationflags)
{
    assert( m_pVertexBuffer == 0 );
    assert( m_pIndexBuffer == 0 );

    unsigned short numverts = 24;
    unsigned int numindices = 36;
    m_NumVertsToDraw = numverts;
    m_NumIndicesToDraw = numindices;

    if( m_pVertexBuffer == 0 )
    {
        m_VertexFormat = VertexFormat_XYZUVNorm;
        Vertex_XYZUVNorm* pVerts = MyNew Vertex_XYZUVNorm[24];
        m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUVNorm)*24, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUVNorm, "MyMesh_Box", "Verts" );
    }

    if( m_pIndexBuffer == 0 )
    {
        unsigned short* pIndices = MyNew unsigned short[36];
        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned short)*36, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 2, "MyMesh_Box", "Indices" );
    }

    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_pVertexBuffer->m_pData;
    unsigned short* pIndices = (unsigned short*)m_pIndexBuffer->m_pData;

    if( pVerts && pIndices )
    {
        float uleft = startu;
        float uright = endu;
        float vtop = startv;
        float vbottom = endv;

        float xleft = 0;
        float xright = boxw;
        float ytop = 0;
        float ybottom = -boxh;
        float zfront = boxd/2;
        float zback = -boxd/2;

        if( justificationflags & Justify_Bottom )
        {
            ytop += boxh;
            ybottom += boxh;
        }
        else if( justificationflags & Justify_CenterY )
        {
            ytop += boxh / 2.0f;
            ybottom += boxh / 2.0f;
        }

        if( justificationflags & Justify_Right )
        {
            xleft -= boxw;
            xright -= boxw;
        }
        else if( justificationflags & Justify_CenterX )
        {
            xleft -= boxw / 2.0f;
            xright -= boxw / 2.0f;
        }

        GLushort side;

        // front
        side = 0;
        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zfront;  pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;    // upper left
        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zfront;  pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;    // upper right
        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zfront;  pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom; // lower left
        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zfront;  pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom; // lower right
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, 0, 1 );

        // back
        side = 1;
        pVerts[4*side + 0] = pVerts[1]; pVerts[4*side + 0].pos.z = zback;  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0]; pVerts[4*side + 1].pos.z = zback;  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3]; pVerts[4*side + 2].pos.z = zback;  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2]; pVerts[4*side + 3].pos.z = zback;  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, 0, -1 );

        // right
        side = 2;
        pVerts[4*side + 0] = pVerts[1];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 1, 0, 0 );

        // left
        side = 3;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( -1, 0, 0 );

        // bottom
        side = 4;
        pVerts[4*side + 0] = pVerts[2];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[3];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, -1, 0 );

        // top
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
    }

    m_MeshReady = true;
}

void MyMesh::CreateBox_XYZUV_RGBA(float boxw, float boxh, float boxd, float startutop, float endutop, float startvtop, float endvtop, float startuside, float enduside, float startvside, float endvside, unsigned char justificationflags)
{
    assert( m_pVertexBuffer == 0 );
    assert( m_pIndexBuffer == 0 );

    unsigned short numverts = 24;
    unsigned int numindices = 36;
    m_NumVertsToDraw = numverts;
    m_NumIndicesToDraw = numindices;

    if( m_pVertexBuffer == 0 )
    {
        m_VertexFormat = VertexFormat_XYZUV_RGBA;
        Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[24];
        m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUV_RGBA)*24, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 2, VertexFormat_XYZUV_RGBA, "MyMesh_BoxXYZUVRGBA", "Verts" );
    }

    if( m_pIndexBuffer == 0 )
    {
        unsigned short* pIndices = MyNew unsigned short[36];
        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned short)*36, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 2, "MyMesh_BoxXYZUVRGBA", "Indices" );
    }

    m_pVertexBuffer->m_Dirty = true;
    m_pIndexBuffer->m_Dirty = true;

    Vertex_XYZUV_RGBA_Alt* pVerts = (Vertex_XYZUV_RGBA_Alt*)m_pVertexBuffer->m_pData;
    unsigned short* pIndices = (unsigned short*)m_pIndexBuffer->m_pData;

    if( pVerts && pIndices )
    {
        float ulefttop = startutop;
        float urighttop = endutop;
        float vtoptop = startvtop;
        float vbottomtop = endvtop;

        float uleftside = startuside;
        float urightside = enduside;
        float vtopside = startvside;
        float vbottomside = endvside;

        float xleft = 0;
        float xright = boxw;
        float ytop = 0;
        float ybottom = -boxh;
        float zfront = boxd/2;
        float zback = -boxd/2;

        if( justificationflags & Justify_Bottom )
        {
            ytop += boxh;
            ybottom += boxh;
        }
        else if( justificationflags & Justify_CenterY )
        {
            ytop += boxh / 2.0f;
            ybottom += boxh / 2.0f;
        }

        if( justificationflags & Justify_Right )
        {
            xleft -= boxw;
            xright -= boxw;
        }
        else if( justificationflags & Justify_CenterX )
        {
            xleft -= boxw / 2.0f;
            xright -= boxw / 2.0f;
        }

        GLushort side;

        // front
        side = 0;
        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zfront;  pVerts[0].uv.x = uleftside;  pVerts[0].uv.y = vtopside;    // upper left
        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zfront;  pVerts[1].uv.x = urightside; pVerts[1].uv.y = vtopside;    // upper right
        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zfront;  pVerts[2].uv.x = uleftside;  pVerts[2].uv.y = vbottomside; // lower left
        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zfront;  pVerts[3].uv.x = urightside; pVerts[3].uv.y = vbottomside; // lower right
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0,1,1 ); //normal.Set( 0, 0, 1 );

        // back
        side = 1;
        pVerts[4*side + 0] = pVerts[1]; pVerts[4*side + 0].pos.z = zback;  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0]; pVerts[4*side + 1].pos.z = zback;  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3]; pVerts[4*side + 2].pos.z = zback;  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2]; pVerts[4*side + 3].pos.z = zback;  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0,0.2f,1 ); //normal.Set( 0, 0, -1 );

        // right
        side = 2;
        pVerts[4*side + 0] = pVerts[1];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //1,0,0,1 ); //normal.Set( 1, 0, 0 );

        // left
        side = 3;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0.2f,0,0,1 ); //normal.Set( -1, 0, 0 );

        // bottom
        side = 4;
        pVerts[4*side + 0] = pVerts[2];  pVerts[4*side + 0].uv.x = ulefttop;  pVerts[4*side + 0].uv.y = vtoptop;
        pVerts[4*side + 1] = pVerts[3];  pVerts[4*side + 1].uv.x = urighttop; pVerts[4*side + 1].uv.y = vtoptop;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv.x = ulefttop;  pVerts[4*side + 2].uv.y = vbottomtop;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv.x = urighttop; pVerts[4*side + 3].uv.y = vbottomtop;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0.2f,0,1 ); //normal.Set( 0, -1, 0 );

        // top
        side = 5;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[4*4 + 0].uv; // copy UV's from "bottom"
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
    }

    m_MeshReady = true;
}

void MyMesh::SetBoxVertexColors(ColorByte TL, ColorByte TR, ColorByte BL, ColorByte BR)
{
    m_pVertexBuffer->m_Dirty = true;

    Vertex_XYZUV_RGBA_Alt* pVerts = (Vertex_XYZUV_RGBA_Alt*)m_pVertexBuffer->m_pData;

    int side;

    // front
    side = 0;
    pVerts[0].col = BL;
    pVerts[1].col = BR;
    pVerts[2].col = BL;
    pVerts[3].col = BR;

    // back
    side = 1;
    pVerts[4*side + 0].col = TR;
    pVerts[4*side + 1].col = TL;
    pVerts[4*side + 2].col = TR;
    pVerts[4*side + 3].col = TL;

    // right
    side = 2;
    pVerts[4*side + 0].col = BR;
    pVerts[4*side + 1].col = TR;
    pVerts[4*side + 2].col = BR;
    pVerts[4*side + 3].col = TR;

    // left
    side = 3;
    pVerts[4*side + 0].col = TL;
    pVerts[4*side + 1].col = BL;
    pVerts[4*side + 2].col = TL;
    pVerts[4*side + 3].col = BL;

    // bottom
    side = 4;
    pVerts[4*side + 0].col = BL;
    pVerts[4*side + 1].col = BR;
    pVerts[4*side + 2].col = TL;
    pVerts[4*side + 3].col = TR;

    // top
    side = 5;
    pVerts[4*side + 0].col = TL;
    pVerts[4*side + 1].col = TR;
    pVerts[4*side + 2].col = BL;
    pVerts[4*side + 3].col = BR;

    m_MeshReady = true;
}

void MyMesh::CreateCylinder(float radius, unsigned short numsegments, float edgeradius, float height, float topstartu, float topendu, float topstartv, float topendv, float sidestartu, float sideendu, float sidestartv, float sideendv)
{
    assert( m_pVertexBuffer == 0 );
    assert( m_pIndexBuffer == 0 );

    float uperc, vperc;

    unsigned short topverts = numsegments + 1;
    unsigned short edgeverts = numsegments * 2;
    unsigned short sideverts = (numsegments + 1) * 2; // +1 since I'm doubling a column of verts to allow mapping from atlas.
    unsigned short numverts = (topverts + edgeverts)*2 + sideverts;
    int numtris = numsegments * 8;

    unsigned int numindices = numtris*3;
    //m_NumVerts = numverts;
    //m_NumIndices = numtris*3;
    m_NumVertsToDraw = numverts;
    m_NumIndicesToDraw = numindices;

    if( m_pVertexBuffer == 0 )
    {
        m_VertexFormat = VertexFormat_XYZUVNorm;
        Vertex_XYZUVNorm* pVerts = MyNew Vertex_XYZUVNorm[numverts];
        m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUVNorm)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUVNorm, "MyMesh_Cylinder", "Verts" );
    }

    if( m_pIndexBuffer == 0 )
    {
        unsigned short* pIndices = MyNew unsigned short[numtris*3];
        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned short)*numtris*3, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 2, "MyMesh_Cylinder", "Indices" );
    }

    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_pVertexBuffer->m_pData;
    unsigned short* pIndices = (unsigned short*)m_pIndexBuffer->m_pData;

    GLushort vertnum = 0;

    float topsizeu = topendu - topstartu;
    float topsizev = topendv - topstartv;

    // create top center vert
    pVerts[vertnum].pos.Set( 0, height, 0 );
    pVerts[vertnum].uv.Set( topstartu + (topsizeu / 2.0f), topstartv + (topsizev / 2.0f) );
    pVerts[vertnum].normal.Set( 0, 1, 0 );
    vertnum++;

    float uvclamplow = 0.01f;
    float uvclamphigh = 1 - uvclamplow;// 0.98f;

    // create top inner and outer(edge) circle verts
    for( int i=0; i<numsegments; i++ )
    {
        float angle = 360.0f/numsegments * i;
        //float nextangle = 360.0f/numsegments * (i+1);

        // inner vert
        pVerts[vertnum].pos.x = sin( angle/360 * 3.1415927f*2 ) * (radius - edgeradius);
        pVerts[vertnum].pos.z = cos( angle/360 * 3.1415927f*2 ) * (radius - edgeradius);
        pVerts[vertnum].pos.y = height;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topstartu + topsizeu * uperc, topstartv + topsizev * vperc );
        pVerts[vertnum].normal.Set( 0, 1, 0 );
        vertnum++;

        // outer vert
        pVerts[vertnum].pos.x = sin( angle/360 * 3.1415927f*2 ) * (radius);
        pVerts[vertnum].pos.z = cos( angle/360 * 3.1415927f*2 ) * (radius);
        pVerts[vertnum].pos.y = height;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topstartu + topsizeu * uperc, topstartv + topsizev * vperc );
        //pVerts[vertnum].normal.Set( 0, 1, 0 );
        pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 1, pVerts[vertnum].pos.z );
        pVerts[vertnum].normal.Normalize();
        vertnum++;
    }

    // create bottom center vert
    pVerts[vertnum].pos.Set( 0, 0, 0 );
    pVerts[vertnum].uv.Set( topstartu + (topsizeu / 2.0f), topstartv + (topsizev / 2.0f) );
    pVerts[vertnum].normal.Set( 0, -1, 0 );
    vertnum++;

    // create bottom inner and outer(edge) circle verts
    for( int i=0; i<numsegments; i++ )
    {
        float angle = 360.0f/numsegments * i;
        //float nextangle = 360.0f/numsegments * (i+i);

        // inner vert
        pVerts[vertnum].pos.x = sin( angle/360 * 3.1415927f*2 ) * (radius - edgeradius);
        pVerts[vertnum].pos.z = cos( angle/360 * 3.1415927f*2 ) * (radius - edgeradius);
        pVerts[vertnum].pos.y = 0;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topendu - topsizeu * uperc, topstartv + topsizev * vperc );
        pVerts[vertnum].normal.Set( 0, -1, 0 );
        vertnum++;

        // outer vert
        pVerts[vertnum].pos.x = sin( angle/360 * 3.1415927f*2 ) * (radius);
        pVerts[vertnum].pos.z = cos( angle/360 * 3.1415927f*2 ) * (radius);
        pVerts[vertnum].pos.y = 0;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topendu - topsizeu * uperc, topstartv + topsizev * vperc );
        //pVerts[vertnum].normal.Set( 0, -1, 0 );
        pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, -1, pVerts[vertnum].pos.z );
        pVerts[vertnum].normal.Normalize();
        vertnum++;
    }

    // create side verts
    GLushort firstsidevert = vertnum;
    {
        int topbase = 0;
        int bottombase = numsegments * 2 + 1;
        for( int i=0; i<numsegments+1; i++ )
        {
            int oldvertindex = i;
            if( i == numsegments )
                oldvertindex = 0;

            // top vert
            pVerts[vertnum].pos.x = pVerts[topbase + oldvertindex*2 + 2].pos.x;
            pVerts[vertnum].pos.z = pVerts[topbase + oldvertindex*2 + 2].pos.z;
            pVerts[vertnum].pos.y = height;
            pVerts[vertnum].uv.x = sidestartu + (sideendu - sidestartu)*i / numsegments;
            pVerts[vertnum].uv.y = sidestartv;// + (sideendv - sidestartv) / 2;
            pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 0, pVerts[vertnum].pos.z );
            pVerts[vertnum].normal.Normalize();
            vertnum++;

            // bottom vert
            pVerts[vertnum].pos.x = pVerts[bottombase + oldvertindex*2 + 2].pos.x;
            pVerts[vertnum].pos.z = pVerts[bottombase + oldvertindex*2 + 2].pos.z;
            pVerts[vertnum].pos.y = 0;
            pVerts[vertnum].uv.x = sidestartu + (sideendu - sidestartu)*i / numsegments;
            pVerts[vertnum].uv.y = sideendv;// + (sideendv - sidestartv) / 2;
            pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 0, pVerts[vertnum].pos.z );
            pVerts[vertnum].normal.Normalize();
            vertnum++;
        }
    }

    int indexnum = 0;

    // setup top center triangles
    if( true )
    {
        GLushort baseindex = 0;
        for( GLushort i=0; i<numsegments; i++ )
        {
            pIndices[indexnum + 0] = 0;
            pIndices[indexnum + 1] = baseindex + i*2 + 1;
            if( i != numsegments-1 )
                pIndices[indexnum + 2] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 2] = baseindex + 1;

            indexnum += 3;
        }
    }

    // setup top outer edge triangles
    if( true )
    {
        for( GLushort i=0; i<numsegments; i++ )
        {
            pIndices[indexnum + 0] = i*2 + 1;
            pIndices[indexnum + 1] = i*2 + 2;
            if( i != numsegments-1 )
                pIndices[indexnum + 2] = (i+1)*2 + 1;
            else
                pIndices[indexnum + 2] = 0*2 + 1;
            indexnum += 3;

            pIndices[indexnum + 0] = (i)*2 + 2;
            if( i != numsegments-1 )
            {
                pIndices[indexnum + 1] = (i+1)*2 + 2;
                pIndices[indexnum + 2] = (i+1)*2 + 1;
            }
            else
            {
                pIndices[indexnum + 1] = (0)*2 + 2;
                pIndices[indexnum + 2] = (0)*2 + 1;
            }
            indexnum += 3;
        }
    }

    // setup side triangles
    if( true )
    {
        for( GLushort i=0; i<numsegments; i++ )
        {
            pIndices[indexnum + 0] = firstsidevert + i*2;
            pIndices[indexnum + 1] = firstsidevert + i*2 + 1;
            pIndices[indexnum + 2] = firstsidevert + i*2 + 2; //(i*2 + 2)%(numsegments*2);
            indexnum += 3;

            pIndices[indexnum + 0] = firstsidevert + i*2 + 1;
            pIndices[indexnum + 1] = firstsidevert + i*2 + 3; //(i*2 + 3)%(numsegments*2);
            pIndices[indexnum + 2] = firstsidevert + i*2 + 2; //(i*2 + 2)%(numsegments*2);
            indexnum += 3;
        }
    }

    // setup bottom center triangles
    if( true )
    {
        GLushort baseindex = (GLushort)(numsegments*2 + 1);
        for( GLushort i=0; i<numsegments; i++ )
        {
            pIndices[indexnum + 0] = baseindex + 0;
            pIndices[indexnum + 2] = baseindex + i*2 + 1;
            if( i != numsegments-1 )
                pIndices[indexnum + 1] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 1] = baseindex + 1;

            indexnum += 3;
        }
    }

    // setup bottom outer edge triangles
    if( true )
    {
        GLushort baseindex = (GLushort)(numsegments*2 + 1);
        for( GLushort i=0; i<numsegments; i++ )
        {
            pIndices[indexnum + 0] = baseindex + i*2 + 1;
            pIndices[indexnum + 2] = baseindex + i*2 + 2;
            if( i != numsegments-1 )
                pIndices[indexnum + 1] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 1] = baseindex + 0*2 + 1;
            indexnum += 3;

            pIndices[indexnum + 0] = baseindex + (i)*2 + 2;
            if( i != numsegments-1 )
            {
                pIndices[indexnum + 2] = baseindex + (i+1)*2 + 2;
                pIndices[indexnum + 1] = baseindex + (i+1)*2 + 1;
            }
            else
            {
                pIndices[indexnum + 2] = baseindex + (0)*2 + 2;
                pIndices[indexnum + 1] = baseindex + (0)*2 + 1;
            }
            indexnum += 3;
        }
    }

    m_MeshReady = true;
}

void MyMesh::CreatePlane(Vector3 topleftpos, Vector2 size, Vector2Int vertcount, Vector2 uvstart, Vector2 uvrange, bool createtriangles)
{
    int numverts = vertcount.x * vertcount.y;
    if( numverts > 65535 )
        return;

    unsigned int numtris = (vertcount.x - 1) * (vertcount.y - 1) * 2;
    m_NumVertsToDraw = (unsigned short)numverts;
    m_NumIndicesToDraw = numtris*3;

    if( m_pVertexBuffer == 0 )
    {
        m_pVertexBuffer = g_pBufferManager->CreateBuffer();
    }

    if( m_pIndexBuffer == 0 && createtriangles )
    {
        m_pIndexBuffer = g_pBufferManager->CreateBuffer();
    }

    // delete the old buffers, if we want a plane with more.
    if( GetNumVerts() < numverts )
    {
        m_pVertexBuffer->FreeBufferedData();
        m_VertexFormat = VertexFormat_XYZUV;
        Vertex_XYZUVNorm* pVerts = MyNew Vertex_XYZUVNorm[numverts];
        m_pVertexBuffer->InitializeBuffer( pVerts, sizeof(Vertex_XYZUVNorm)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUV, "MyMesh_Plane", "Verts" );

        if( createtriangles )
        {
            m_pIndexBuffer->FreeBufferedData();
            unsigned short* pIndices = MyNew unsigned short[numtris*3];
            m_pIndexBuffer->InitializeBuffer( pIndices, sizeof(unsigned short)*numtris*3, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 2, "MyMesh_Plane", "Indices" );
        }
    }

    Vertex_XYZUV* pVerts = (Vertex_XYZUV*)m_pVertexBuffer->m_pData;
    m_pVertexBuffer->m_Dirty = true;

    unsigned short* pIndices = 0;
    if( createtriangles )
    {
        pIndices = (unsigned short*)m_pIndexBuffer->m_pData;
        m_pIndexBuffer->m_Dirty = true;
    }

    for( int y = 0; y < vertcount.y; y++ )
    {
        for( int x = 0; x < vertcount.x; x++ )
        {
            int index = y * vertcount.x + x;

            pVerts[index].x = topleftpos.x + size.x / (vertcount.x - 1) * x;
            pVerts[index].y = topleftpos.y;
            pVerts[index].z = topleftpos.z + size.y / (vertcount.y - 1) * y;

            pVerts[index].u = uvstart.x + x * uvrange.x / (vertcount.x - 1);
            pVerts[index].v = uvstart.y + y * uvrange.y / (vertcount.y - 1);
        }
    }

    if( createtriangles )
    {
        for( int y = 0; y < vertcount.y - 1; y++ )
        {
            for( int x = 0; x < vertcount.x - 1; x++ )
            {
                int elementindex = (y * (vertcount.x-1) + x) * 6;
                unsigned short vertexindex = (unsigned short)(y * vertcount.x + x);

                pIndices[ elementindex + 0 ] = vertexindex + 0;
                pIndices[ elementindex + 1 ] = vertexindex + (unsigned short)vertcount.x;
                pIndices[ elementindex + 2 ] = vertexindex + 1;

                pIndices[ elementindex + 3 ] = vertexindex + 1;
                pIndices[ elementindex + 4 ] = vertexindex + (unsigned short)vertcount.x;
                pIndices[ elementindex + 5 ] = vertexindex + (unsigned short)vertcount.x + 1;
            }
        }
    }

    m_MeshReady = true;
};

void MyMesh::CreateEditorLineGridXZ(Vector3 center, float spacing, int halfnumbars)
{
    assert( m_pVertexBuffer == 0 );
    assert( m_pIndexBuffer == 0 );

    unsigned char numverts = (unsigned char)((halfnumbars*2+1) * 2 * 2);
    unsigned int numindices = (halfnumbars*2+1) * 2 * 2; // halfnumbars*2+1centerline * 2axis * 2indicesperline.
    m_NumVertsToDraw = numverts; // not optimizing reuse of corner verts.
    m_NumIndicesToDraw = numindices;

    if( m_pVertexBuffer == 0 )
    {
        m_VertexFormat = VertexFormat_XYZ;
        Vertex_XYZ* pVerts = MyNew Vertex_XYZ[numverts];
        m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZ)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZ, "MyMesh_GridPlane", "Verts" );
    }

    if( m_pIndexBuffer == 0 )
    {
        unsigned char* pIndices = MyNew unsigned char[numindices];
        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned char)*numindices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 1, "MyMesh_GridPlane", "Indices" );
    }

    m_PrimitiveType = GL_LINES;

    Vertex_XYZ* pVerts = (Vertex_XYZ*)m_pVertexBuffer->m_pData;
    unsigned char* pIndices = (unsigned char*)m_pIndexBuffer->m_pData;

    unsigned char vertnum = 0;
    int indexnum = 0;

    // create verts
    for( int i = -halfnumbars; i<=halfnumbars; i++ )
    {
        pVerts[vertnum].x = center.x + i*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + halfnumbars*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + i*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + -halfnumbars*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + halfnumbars*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + i*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + -halfnumbars*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + i*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;
    }

    m_MeshReady = true;
}

void MyMesh::CreateEditorTransformGizmoAxis(float length, float thickness, ColorByte color)
{
    assert( m_pVertexBuffer == 0 );
    assert( m_pIndexBuffer == 0 );

    int numsides = 4;
    unsigned char numverts = (unsigned char)(2 * (numsides+1));
    unsigned int numindices = 6 * numsides;
    m_NumVertsToDraw = numverts;
    m_NumIndicesToDraw = numindices;

    m_Tint = color;

    if( m_pVertexBuffer == 0 )
    {
        m_VertexFormat = VertexFormat_XYZ;
        Vertex_XYZ* pVerts = MyNew Vertex_XYZ[numverts];
        m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZ)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZ, "MyMesh_TransformAxis", "Verts" );
    }

    if( m_pIndexBuffer == 0 )
    {
        unsigned char* pIndices = MyNew unsigned char[numindices];
        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned char)*numindices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 1, "MyMesh_TransformAxis", "Indices" );
    }

    m_PrimitiveType = GL_TRIANGLES;

    Vertex_XYZ* pVerts = (Vertex_XYZ*)m_pVertexBuffer->m_pData;
    unsigned char* pIndices = (unsigned char*)m_pIndexBuffer->m_pData;

    unsigned char vertnum = 0;
    int indexnum = 0;

    // create verts
    for( int i=0; i<=numsides; i++ )
    {
        float angle = 2*PI / numsides * i;

        pVerts[vertnum + 0].x = cos( angle ) * thickness;
        pVerts[vertnum + 0].y = sin( angle ) * thickness;
        pVerts[vertnum + 0].z = 0;

        pVerts[vertnum + 1].x = cos( angle ) * thickness;
        pVerts[vertnum + 1].y = sin( angle ) * thickness;
        pVerts[vertnum + 1].z = -length;

        if( i < numsides )
        {
            pIndices[indexnum + 0] = vertnum + 2;
            pIndices[indexnum + 1] = vertnum + 0;
            pIndices[indexnum + 2] = vertnum + 1;
            pIndices[indexnum + 3] = vertnum + 2;
            pIndices[indexnum + 4] = vertnum + 1;
            pIndices[indexnum + 5] = vertnum + 3;
        }

        vertnum += 2;
        indexnum += 6;
    }

    m_MeshReady = true;
}

void MyMesh::SetShaderGroup(ShaderGroup* pShaderGroup)
{
    m_pShaderGroup = pShaderGroup;

    // rebuild the vaos in case the attributes required for the shader are different than the last shader assigned.
    if( m_pVertexBuffer )
        m_pVertexBuffer->ResetVAOs();
}

void MyMesh::SetShaderAndTexture(ShaderGroup* pShaderGroup, TextureDefinition* pTexture)
{
    m_pShaderGroup = pShaderGroup;
    m_pTexture = pTexture;
}

void MyMesh::SetTextureProperties(ColorByte tint, ColorByte speccolor, float shininess)
{
    m_Tint = tint;
    m_SpecColor = speccolor;
    m_Shininess = shininess;
}

void MyMesh::SetPosition(float x, float y, float z)
{
    m_Position.SetTranslation( x, y, z );
}

void MyMesh::RebuildIndices()
{
    m_pIndexBuffer->Rebuild( 0, m_pIndexBuffer->m_DataSize );
}

void MyMesh::Draw(MyMatrix* matviewproj, Vector3* campos, MyLight* lights, int numlights, MyMatrix* shadowlightwvp, int shadowtexid, int lightmaptexid, ShaderGroup* pShaderOverride)
{
    if( m_MeshReady == false )
    {
        if( strcmp( m_pSourceFile->m_ExtensionWithDot, ".obj" ) == 0 )
        {
            CreateFromOBJFile( m_pSourceFile );
        }
        return;
    }

    if( m_pShaderGroup == 0 )
        return;

    if( m_NumIndicesToDraw == 0 )
        return;

    assert( m_pVertexBuffer );

    if( m_pVertexBuffer->m_Dirty )
        m_pVertexBuffer->Rebuild( 0, m_NumVertsToDraw*g_VertexFormatSizes[m_VertexFormat] );
    if( m_pIndexBuffer && m_pIndexBuffer->m_Dirty )
        m_pIndexBuffer->Rebuild( 0, m_NumIndicesToDraw*m_pIndexBuffer->m_BytesPerIndex );
    assert( ( m_pIndexBuffer == 0 || m_pIndexBuffer->m_Dirty == false ) && m_pVertexBuffer->m_Dirty == false );

    checkGlError( "Drawing Mesh Rebuild()" );

    if( pShaderOverride )
    {
        int indexbuffertype = GL_UNSIGNED_BYTE;
        if( m_pIndexBuffer != 0 )
        {
            int bytesperindex = m_pIndexBuffer->m_BytesPerIndex;
            if( bytesperindex == 2 )
                indexbuffertype = GL_UNSIGNED_SHORT;
            else if( bytesperindex == 4 )
                indexbuffertype = GL_UNSIGNED_INT;
        }

        Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass();
        pShader->SetupAttributes( (VertexFormats)m_VertexFormat, m_pVertexBuffer, m_pIndexBuffer, false );
        pShader->ProgramPosition( matviewproj, &m_Position );

        if( m_pIndexBuffer )
            MyDrawElements( m_PrimitiveType, m_NumIndicesToDraw, indexbuffertype, 0 );
        else
            MyDrawArrays( m_PrimitiveType, 0, m_NumIndicesToDraw );
        //pShader->DeactivateShader( m_pVertexBuffer ); // disable attributes
    }
    else
    {
        Shader_Base* pShader = (Shader_Base*)m_pShaderGroup->GlobalPass();
        if( pShader )
        {
            if( pShader->ActivateAndProgramShader( (VertexFormats)m_VertexFormat,
                m_pVertexBuffer, m_pIndexBuffer, GL_UNSIGNED_SHORT,
                matviewproj, &m_Position, m_pTexture ? m_pTexture->m_TextureID : 0, m_Tint, m_SpecColor, m_Shininess ) )
            {
                checkGlError( "Drawing Mesh ActivateAndProgramShader()" );

                pShader->ProgramCamera( campos );
                checkGlError( "Drawing Mesh ProgramCamera()" );

                pShader->ProgramLights( lights, numlights );
                checkGlError( "Drawing Mesh ProgramCamera()" );

                if( m_PrimitiveType == GL_POINTS )
                    pShader->ProgramPointSize( (float)m_PointSize );

                if( shadowlightwvp && shadowtexid != 0 )
                    pShader->ProgramShadowLight( &m_Position, shadowlightwvp, shadowtexid );

                if( lightmaptexid != 0 )
                {
                    pShader->ProgramLightmap( lightmaptexid );
                    checkGlError( "Drawing Mesh ProgramLightmap()" );
                }

                int indexbuffertype = GL_UNSIGNED_BYTE;
                if( m_pIndexBuffer != 0 )
                {
                    int bytesperindex = m_pIndexBuffer->m_BytesPerIndex;
                    if( bytesperindex == 2 )
                        indexbuffertype = GL_UNSIGNED_SHORT;
                    else if( bytesperindex == 4 )
                        indexbuffertype = GL_UNSIGNED_INT;
                }

                if( m_pIndexBuffer )
                    MyDrawElements( m_PrimitiveType, m_NumIndicesToDraw, indexbuffertype, 0 );
                else
                    MyDrawArrays( m_PrimitiveType, 0, m_NumIndicesToDraw );

                checkGlError( "Drawing Mesh MyDrawElements()" );

                pShader->DeactivateShader( m_pVertexBuffer );
                checkGlError( "Drawing Mesh DeactivateShader()" );
            }
        }
    }
}

unsigned short MyMesh::GetNumVerts()
{
    if( m_pVertexBuffer == 0 )
        return 0;

    return (unsigned short)(m_pVertexBuffer->m_DataSize / g_VertexFormatSizes[m_pVertexBuffer->m_VertexFormat]);
}

unsigned int MyMesh::GetNumIndices()
{
    if( m_pIndexBuffer == 0 )
        return 0;

    return m_pIndexBuffer->m_DataSize / m_pIndexBuffer->m_BytesPerIndex;
}

Vertex_Base* MyMesh::GetVerts(bool markdirty)
{
    if( markdirty )
        m_pVertexBuffer->m_Dirty = true;

    return (Vertex_Base*)m_pVertexBuffer->m_pData;
}

unsigned short* MyMesh::GetIndices(bool markdirty)
{
    if( markdirty )
        m_pIndexBuffer->m_Dirty = true;

    return (unsigned short*)m_pIndexBuffer->m_pData;
}

unsigned int MyMesh::GetStride()
{
    return g_VertexFormatSizes[m_pVertexBuffer->m_VertexFormat];
}
