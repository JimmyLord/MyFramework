//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "ParticleRenderer.h"

#define USE_INDEXED_TRIANGLES   1

ParticleRenderer::ParticleRenderer()
{
    m_pMaterial = g_pMaterialManager->CreateMaterial();
    m_NumVertsAllocated = 0;

    m_pVertexBuffer = 0;
    m_pIndexBuffer = 0;

    m_ParticleCount = 0;

    m_Additive = true;

    m_ScalePosToScreenSize = true;
    m_2DCameraZoom = 1.0f;

    m_ScreenOffsetX = 0;
    m_ScreenOffsetY = 0;
    m_GameWidth = 0;
    m_GameHeight = 0;
    m_DeviceWidth = 0;
    m_DeviceHeight = 0;

    //m_Position.SetIdentity();
}

ParticleRenderer::~ParticleRenderer()
{
    SAFE_RELEASE( m_pVertexBuffer );
    SAFE_RELEASE( m_pIndexBuffer );

    SAFE_RELEASE( m_pMaterial );
}

void ParticleRenderer::AllocateVertices(unsigned int numpoints, const char* category)
{
    LOGInfo( LOGTag, "ParticleRenderer: Allocating %d Verts\n", numpoints );

    assert( m_pVertexBuffer == 0 );

#if USE_INDEXED_TRIANGLES
    unsigned int numverts = numpoints * 4;
    unsigned int numindices = numpoints * 6;
#else
    unsigned int numverts = numpoints;
    unsigned int numindices = 0;
#endif

    Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[numverts];
    m_NumVertsAllocated = numverts;

    LOGInfo( LOGTag, "ParticleRenderer: about to call glGenBuffers\n" );

    m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUV_RGBA)*numverts, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, true, 2, VertexFormat_XYZUV_RGBA, category, "Particles-Verts" );

    LOGInfo( LOGTag, "ParticleRenderer: m_pVertexBuffer->m_BufferID = %d\n", m_pVertexBuffer->m_CurrentBufferID );

    if( numindices != 0 )
    {
        GLushort* tempindices = MyNew GLushort[numindices]; // deleted eventually by buffermanager/bufferdef
        for( GLushort i=0; i<numpoints; i++ )
        {
#if !USE_D3D
            // Counter-clockwise
            tempindices[i*6+0] = i*4+0;
            tempindices[i*6+1] = i*4+2;
            tempindices[i*6+2] = i*4+1;
            tempindices[i*6+3] = i*4+2;
            tempindices[i*6+4] = i*4+3;
            tempindices[i*6+5] = i*4+1;
#else
            // This is insane, should be just changing cull winding order
            // fix if d3d is ever tested again.
            assert( false );
            // clockwise
            tempindices[i*6+0] = i*4+0;
            tempindices[i*6+1] = i*4+1;
            tempindices[i*6+2] = i*4+2;
            tempindices[i*6+3] = i*4+2;
            tempindices[i*6+4] = i*4+1;
            tempindices[i*6+5] = i*4+3;
#endif
        }

        m_pIndexBuffer = g_pBufferManager->CreateBuffer( tempindices, sizeof(unsigned short)*numindices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, 2, category, "Particles-Indices" );
    }

    //m_pVAO = g_pBufferManager->CreateVAO();
}

void ParticleRenderer::AddPoint(Vector2 pos, float rot, ColorByte color, float size)
{
    AddPoint( Vector3( pos.x, pos.y, 0 ), rot, color, size );
}

void ParticleRenderer::AddPoint(Vector3 pos, float rot, ColorByte color, float size)
{
    //assert( m_ParticleCount < m_NumVertsAllocated );
    unsigned int vertexnum = m_ParticleCount;

#if USE_INDEXED_TRIANGLES
    vertexnum *= 4;
#endif

    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->m_pData;
    m_pVertexBuffer->m_Dirty = true;

    if( vertexnum < m_NumVertsAllocated )
    {
#if USE_INDEXED_TRIANGLES
        {
            pVerts[vertexnum].x = pos.x;
            pVerts[vertexnum].y = pos.y;
            pVerts[vertexnum].z = pos.z;

            pVerts[vertexnum].r = color.r;
            pVerts[vertexnum].g = color.g;
            pVerts[vertexnum].b = color.b;
            pVerts[vertexnum].a = color.a;

            //pVerts[vertexnum].size = size * m_2DCameraZoom;

            pVerts[vertexnum+1] = pVerts[vertexnum];
            pVerts[vertexnum+2] = pVerts[vertexnum];
            pVerts[vertexnum+3] = pVerts[vertexnum];

            float halfsize = size / 2;
            pVerts[vertexnum+0].x -= halfsize;
            pVerts[vertexnum+0].y += halfsize;
            pVerts[vertexnum+0].u = 0;
            pVerts[vertexnum+0].v = 0;

            pVerts[vertexnum+1].x += halfsize;
            pVerts[vertexnum+1].y += halfsize;
            pVerts[vertexnum+1].u = 1;
            pVerts[vertexnum+1].v = 0;

            pVerts[vertexnum+2].x -= halfsize;
            pVerts[vertexnum+2].y -= halfsize;
            pVerts[vertexnum+2].u = 0;
            pVerts[vertexnum+2].v = 1;

            pVerts[vertexnum+3].x += halfsize;
            pVerts[vertexnum+3].y -= halfsize;
            pVerts[vertexnum+3].u = 1;
            pVerts[vertexnum+3].v = 1;
        }
#else
        {
            assert( false ); // not supported
            float screenw = (float)g_pGameCore->m_WindowWidth;
            float screenh = (float)g_pGameCore->m_WindowHeight;

            float screenscale = screenw / m_GameWidth;
            float screenscaleh = screenh / m_GameHeight;
            if( screenscaleh < screenscale )
                screenscale = screenscaleh;
            if( screenscale > 1 )
                screenscale = 1;

            if( m_ScalePosToScreenSize )
            {
                float screenspacex = m_DeviceWidth - m_ScreenOffsetX*2;
                float screenspacey = m_DeviceHeight - m_ScreenOffsetY*2;

                pVerts[vertexnum].x = (m_ScreenOffsetX+((pos.x/m_GameWidth)*screenspacex))/m_DeviceWidth * 2 - 1.0f;
                pVerts[vertexnum].y = (m_ScreenOffsetY+((pos.y/m_GameHeight)*screenspacey))/m_DeviceHeight * 2 - 1.0f;
            }
            else
            {
                pVerts[vertexnum].x = pos.x;
                pVerts[vertexnum].y = pos.y;
            }
            //pVerts[vertexnum].rot = rot;
            pVerts[vertexnum].r = color.r;
            pVerts[vertexnum].g = color.g;
            pVerts[vertexnum].b = color.b;
            pVerts[vertexnum].a = color.a;
            pVerts[vertexnum].size = size * screenscale * m_2DCameraZoom;
        }
#endif

        m_ParticleCount++;
    }
}

void ParticleRenderer::SetShaderAndTexture(ShaderGroup* pShaderGroup, TextureDefinition* pTexture)
{
    m_pMaterial->SetShader( pShaderGroup );
    m_pMaterial->SetTextureColor( pTexture );

    m_pVertexBuffer->ResetVAOs();
}

void ParticleRenderer::Draw(MyMatrix* matviewproj)
{
#if MY_SHITTY_LAPTOP
    //return;
#endif

    if( m_pMaterial == 0 || m_pMaterial->GetTextureColor() == 0 || m_pMaterial->GetTextureColor()->m_TextureID == 0 ||
        m_pMaterial->GetShader() == 0 || m_ParticleCount == 0 )
        return;

    //glEnable(GL_TEXTURE_2D);
#if MYFW_WINDOWS
//#define GL_POINT_SPRITE 0x8861
//#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642

#if !USE_INDEXED_TRIANGLES
    glEnable( GL_POINT_SPRITE );
    glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
#endif
#endif
    //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    //glEnable(GL_ARB_point_sprite);

    //glEnable( GL_BLEND );
    if( m_Additive )
    {
        glBlendFunc( GL_ONE, GL_ONE );
#if USE_D3D
        float blendfactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        g_pD3DContext->OMSetBlendState( g_pD3DBlendStateEnabledAdditive.Get(), blendfactor, 0xfff);
#endif
    }

    int numverts = m_ParticleCount;
#if USE_INDEXED_TRIANGLES
    numverts *= 4;
#endif

    if( m_pVertexBuffer->m_Dirty )
        m_pVertexBuffer->Rebuild( 0, sizeof(Vertex_XYZUV_RGBA)*numverts );
    if( m_pIndexBuffer->m_Dirty )
        m_pIndexBuffer->Rebuild( 0, m_pIndexBuffer->m_DataSize );
    assert( m_pIndexBuffer->m_Dirty == false && m_pVertexBuffer->m_Dirty == false );

#if USE_INDEXED_TRIANGLES
    if( ((Shader_Base*)m_pMaterial->GetShader()->GlobalPass())->ActivateAndProgramShader(
        m_pVertexBuffer, m_pIndexBuffer, GL_UNSIGNED_SHORT,
        matviewproj, 0, m_pMaterial ) )
    {
        MyDrawElements( GL_TRIANGLES, m_ParticleCount*6, GL_UNSIGNED_SHORT, 0 );
        m_pMaterial->GetShader()->GlobalPass()->DeactivateShader( m_pVertexBuffer );
    }
#else
    // not supporting point sprites anymore.
    assert( false );
    //if( ((Shader_PointSprite*)m_pShaderGroup->GlobalPass())->ActivateAndProgramShader( 
    //    matviewproj, 0, m_VertexBufferID, 0, GL_UNSIGNED_SHORT, m_pTexture->m_TextureID ) )
    //{
    //    MyDrawArrays( GL_POINTS, 0, m_ParticleCount );
    //    m_pMaterial->m_pShaderGroup->GlobalPass()->DeactivateShader();
    //}
#endif

    //glEnable( GL_BLEND );
    if( m_Additive ) // revert back to regular enabled alpha blending.
    {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#if USE_D3D
        float blendfactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        g_pD3DContext->OMSetBlendState( g_pD3DBlendStateEnabled.Get(), blendfactor, 0xfff);
#endif
    }

    return;
}
