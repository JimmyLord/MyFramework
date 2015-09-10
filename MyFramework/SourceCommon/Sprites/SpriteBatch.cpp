//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "SpriteBatch.h"

SpriteBatch::SpriteBatch()
{
    m_pMaterial = 0;//g_pMaterialManager->CreateMaterial();
    m_NumSprites = 0;

    m_SpritesAllocated = 0;
    m_pVertexBuffer = 0;
    m_pIndexBuffer = 0;
    //m_pVAO = 0;
}

SpriteBatch::~SpriteBatch()
{
    SAFE_RELEASE( m_pVertexBuffer );
    SAFE_RELEASE( m_pIndexBuffer );

    SAFE_RELEASE( m_pMaterial );
    //SAFE_RELEASE( m_pVAO );
}

void SpriteBatch::SetMaterial(MaterialDefinition* pMaterial)
{
    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;
}

void SpriteBatch::AllocateVertices(int numsprites)
{
    MyAssert( m_pVertexBuffer == 0 );

    m_SpritesAllocated = numsprites;

    Vertex_Base* pVerts = (Vertex_Base*)MyNew Vertex_Sprite[numsprites*4];
    MyAssert( pVerts );

    GLushort* pIndices = MyNew GLushort[numsprites*6];
    MyAssert( pIndices );

    // allocate 2 empty buffers, will be filled by subbufferdata elsewhere.
    m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_Sprite)*numsprites*4, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, false, 2, VertexFormat_Sprite, "SpriteBatch", "Verts" );

    for( GLushort i=0; i<numsprites; i++ )
    {
#if MYFW_RIGHTHANDED
        pIndices[i*6 + 0] = i*4 + 0;
        pIndices[i*6 + 1] = i*4 + 1;
        pIndices[i*6 + 2] = i*4 + 2;
        pIndices[i*6 + 3] = i*4 + 2;
        pIndices[i*6 + 4] = i*4 + 1;
        pIndices[i*6 + 5] = i*4 + 3;
#else
        pIndices[i*6 + 0] = i*4 + 0;
        pIndices[i*6 + 1] = i*4 + 2;
        pIndices[i*6 + 2] = i*4 + 1;
        pIndices[i*6 + 3] = i*4 + 2;
        pIndices[i*6 + 4] = i*4 + 3;
        pIndices[i*6 + 5] = i*4 + 1;
#endif
    }

    m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(GLushort)*numsprites*6, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, VertexFormat_None, "SpriteBatch", "Indices" );

    //m_pVAO = g_pBufferManager->CreateVAO();
}

void SpriteBatch::AddSprite(MySprite* pSprite)
{
    Vertex_Sprite* pSpriteVerts = (Vertex_Sprite*)pSprite->GetVerts( false );
    MyMatrix spriteTransform = pSprite->GetPosition();
    Vertex_Sprite* pBatchVerts = (Vertex_Sprite*)m_pVertexBuffer->m_pData;

    MyAssert( m_NumSprites < m_SpritesAllocated - 1 );
    if( m_NumSprites >= m_SpritesAllocated - 1 )
        return;

    for( int i=0; i<4; i++ )
    {
        //Vector2 pos( pSpriteVerts[0+i].x, pSpriteVerts[0+i].y );
        //pBatchVerts[m_NumSprites*4 + i] = spriteTransform * pos;

        pBatchVerts[m_NumSprites*4 + i] = pSpriteVerts[0+i];
        *((Vector2*)(&pBatchVerts[m_NumSprites*4 + i].x)) = spriteTransform * (*((Vector2*)(&pSpriteVerts[0+i].x)));
        
        //pBatchVerts[m_NumSprites*4 + i] = pSpriteVerts[0+i];
        //pBatchVerts[m_NumSprites*4 + i].x += spriteTransform.m41;
        //pBatchVerts[m_NumSprites*4 + i].y += spriteTransform.m42;
    }

    m_pVertexBuffer->m_Dirty = true;

    m_NumSprites++;
}

void SpriteBatch::Draw(MyMatrix* matviewproj)
{
    if( m_pMaterial == 0 || m_pMaterial->GetTextureColor() == 0 || m_pMaterial->GetTextureColor()->m_TextureID == 0 ||
        m_pMaterial->GetShader() == 0 || m_NumSprites == 0 )
        return;

    MyMatrix pos;
    pos.SetIdentity();

    if( m_pVertexBuffer->m_Dirty )
        m_pVertexBuffer->Rebuild( 0, sizeof(Vertex_Sprite)*m_NumSprites*4 );
    if( m_pIndexBuffer->m_Dirty )
        m_pIndexBuffer->Rebuild( 0, m_pIndexBuffer->m_DataSize );
    MyAssert( m_pIndexBuffer->m_Dirty == false && m_pVertexBuffer->m_Dirty == false );

    //// update the vertex buffer with new verts data.
    //MyBindBuffer( GL_ARRAY_BUFFER, m_pVertexBuffer->m_CurrentBufferID );
    //glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(Vertex_Sprite)*m_NumSprites*4, (void*)m_pVerts );
    ////glBufferData( GL_ARRAY_BUFFER, sizeof(Vertex_Sprite)*m_NumSprites*4, (void*)m_pVerts, GL_DYNAMIC_DRAW );

    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();
    if( pShader == 0 )
        return;

    // Enable blending if necessary. TODO: sort draws and only set this once.
    if( m_pMaterial->IsTransparent( pShader ) )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    // Draw the contents of the buffers.
    if( pShader->ActivateAndProgramShader(
            m_pVertexBuffer, m_pIndexBuffer, GL_UNSIGNED_SHORT,
            matviewproj, &pos, m_pMaterial ) )
    {
#if USE_D3D
        g_pD3DContext->DrawIndexed( m_NumSprites*6, 0, 0 );
#else
        MyDrawElements( GL_TRIANGLES, m_NumSprites*6, GL_UNSIGNED_SHORT, 0 );
#endif
        pShader->DeactivateShader( m_pVertexBuffer );
    }

    // always disable blending
    glDisable( GL_BLEND );

    return;
}
