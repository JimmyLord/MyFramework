//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "SpriteBatch_XYZVertexColor.h"

SpriteBatch_XYZVertexColor::SpriteBatch_XYZVertexColor()
{
}

SpriteBatch_XYZVertexColor::~SpriteBatch_XYZVertexColor()
{
}

void SpriteBatch_XYZVertexColor::AllocateVertices(int numsprites)
{
    assert( m_pVertexBuffer == 0 );

    m_SpritesAllocated = numsprites;

    Vertex_Base* pVerts = (Vertex_Base*)MyNew Vertex_XYZUV_RGBA[numsprites*4];
    assert( pVerts );

    GLushort* pIndices = MyNew GLushort[numsprites*6];
    assert( pIndices );

    // allocate 2 empty buffers, will be filled by subbufferdata elsewhere.
    m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUV_RGBA)*numsprites*4, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, false, 2, VertexFormat_XYZUV_RGBA, "SpriteBatch_XYZVertexColor", "Verts" );

    for( GLushort i=0; i<numsprites; i++ )
    {
        pIndices[i*6 + 0] = i*4 + 0;
        pIndices[i*6 + 1] = i*4 + 1;
        pIndices[i*6 + 2] = i*4 + 2;
        pIndices[i*6 + 3] = i*4 + 2;
        pIndices[i*6 + 4] = i*4 + 1;
        pIndices[i*6 + 5] = i*4 + 3;
    }

    m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(GLushort)*numsprites*6, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, VertexFormat_None, "SpriteBatch_XYZVertexColor", "Indices" );

    //m_pVAO = g_pBufferManager->CreateVAO();
}

void SpriteBatch_XYZVertexColor::AddSprite(MySprite* pSprite)
{
    AddSprite( (MySprite_XYZVertexColor*)pSprite );
}

void SpriteBatch_XYZVertexColor::AddSprite(MySprite_XYZVertexColor* pSprite)
{
    Vertex_XYZUV_RGBA* pSpriteVerts = (Vertex_XYZUV_RGBA*)pSprite->GetVerts( false );
    //MyMatrix spriteTransform = pSprite->GetPosition();
    Vertex_XYZUV_RGBA* pBatchVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->m_pData;

    //assert( m_NumSprites < m_SpritesAllocated - 1 );
    if( m_NumSprites >= m_SpritesAllocated - 1 )
    {
        LOGInfo( LOGTag, "m_NumSprites >= m_SpritesAllocated - 1\n" );
        return;
    }

    for( int i=0; i<4; i++ )
    {
        pBatchVerts[m_NumSprites*4 + i] = pSpriteVerts[0+i];

        Vertex_XYZUV_RGBA* vertorig = &pBatchVerts[m_NumSprites*4 + i];
        Vector3 vertcopy = Vector3( vertorig->x, vertorig->y, vertorig->z );

        Vector3 rotatedvert = pSprite->GetPosition() * vertcopy;

        pBatchVerts[m_NumSprites*4 + i].x = rotatedvert.x;
        pBatchVerts[m_NumSprites*4 + i].y = rotatedvert.y;
        pBatchVerts[m_NumSprites*4 + i].z = rotatedvert.z;
        //pBatchVerts[m_NumSprites*4 + i].x += spriteTransform.m41;
        //pBatchVerts[m_NumSprites*4 + i].y += spriteTransform.m42;
        //pBatchVerts[m_NumSprites*4 + i].z += spriteTransform.m43;

        pBatchVerts[m_NumSprites*4 + i].r = (unsigned char)(pBatchVerts[m_NumSprites*4 + i].r * pSprite->m_Tint.r / 255.0f);
        pBatchVerts[m_NumSprites*4 + i].g = (unsigned char)(pBatchVerts[m_NumSprites*4 + i].g * pSprite->m_Tint.g / 255.0f);
        pBatchVerts[m_NumSprites*4 + i].b = (unsigned char)(pBatchVerts[m_NumSprites*4 + i].b * pSprite->m_Tint.b / 255.0f);
        pBatchVerts[m_NumSprites*4 + i].a = (unsigned char)(pBatchVerts[m_NumSprites*4 + i].a * pSprite->m_Tint.a / 255.0f);
    }

    m_pVertexBuffer->m_Dirty = true;

    m_NumSprites++;
}

void SpriteBatch_XYZVertexColor::Draw(MyMatrix* matviewproj)
{
    if( m_pTexture == 0 || m_pTexture->m_TextureID == 0 || m_pShaderGroup == 0 || m_NumSprites == 0 )
        return;

    MyMatrix pos;
    pos.SetIdentity();

    if( m_pVertexBuffer->m_Dirty )
        m_pVertexBuffer->Rebuild( 0, sizeof(Vertex_XYZUV_RGBA)*m_NumSprites*4 );
    if( m_pIndexBuffer->m_Dirty )
        m_pIndexBuffer->Rebuild( 0, m_pIndexBuffer->m_DataSize );
    assert( m_pIndexBuffer->m_Dirty == false && m_pVertexBuffer->m_Dirty == false );

    checkGlError( "pre MyBindBuffer" );

    //// update the vertex buffer with new verts data.
    //MyBindBuffer( GL_ARRAY_BUFFER, m_pVertexBuffer->m_CurrentBufferID );
    //checkGlError( "MyBindBuffer" );
    //glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(Vertex_XYZUV_RGBA)*m_NumSprites*4, (void*)m_pVerts );
    //checkGlError( "glBufferSubData" );
    //MyBindBuffer( GL_ARRAY_BUFFER, 0 );
    //checkGlError( "MyBindBuffer" );

    // Draw the contents of the buffers.
    if( ((Shader_Base*)m_pShaderGroup->GlobalPass())->ActivateAndProgramShader(
            m_pVertexBuffer, m_pIndexBuffer, GL_UNSIGNED_SHORT,
            matviewproj, &pos, m_pTexture ) )
    {
#if USE_D3D
        g_pD3DContext->DrawIndexed( m_NumSprites*6, 0, 0 );
#else
        MyDrawElements( GL_TRIANGLES, m_NumSprites*6, GL_UNSIGNED_SHORT, 0 );
#endif
        m_pShaderGroup->GlobalPass()->DeactivateShader( m_pVertexBuffer );
    }

    return;
}
