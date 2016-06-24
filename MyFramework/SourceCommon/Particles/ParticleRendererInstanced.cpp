//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#if MYFW_USEINSTANCEDPARTICLES

#include "ParticleRendererInstanced.h"

ParticleRendererInstanced::ParticleRendererInstanced(bool creatematerial)
: ParticleRenderer( creatematerial )
{
    m_ParticleDataBuffer = 0;
    m_NumParticlesAllocated = 0;

    m_pParticleData = 0;

    m_pParticleQuadVerts = 0;
}

ParticleRendererInstanced::~ParticleRendererInstanced()
{
    delete[] m_pParticleData;
    glDeleteBuffers( 1, &m_ParticleDataBuffer );

    // m_pParticleQuadVerts; // will be deleted by VBO.
}

void ParticleRendererInstanced::AllocateVertices(unsigned int numpoints, const char* category)
{
    LOGInfo( LOGTag, "ParticleRendererInstanced: Allocating %d Verts\n", numpoints );

    MyAssert( m_pVertexBuffer == 0 );

    unsigned int numverts = 4;

    m_pParticleData = new ParticleInstanceData[numpoints];
    m_NumParticlesAllocated = numpoints;
    glGenBuffers( 1, &m_ParticleDataBuffer );

    Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[numverts];
    m_pParticleQuadVerts = pVerts;
    float halfsize = 0.5;
    pVerts[0].x = -halfsize; pVerts[0].y = +halfsize; pVerts[0].z = 0; pVerts[0].u = 0; pVerts[0].v = 0;
    pVerts[1].x = +halfsize; pVerts[1].y = +halfsize; pVerts[1].z = 0; pVerts[1].u = 1; pVerts[1].v = 0;
    pVerts[2].x = -halfsize; pVerts[2].y = -halfsize; pVerts[2].z = 0; pVerts[2].u = 0; pVerts[2].v = 1;
    pVerts[3].x = +halfsize; pVerts[3].y = -halfsize; pVerts[3].z = 0; pVerts[3].u = 1; pVerts[3].v = 1;
    m_NumVertsAllocated = numverts;

    m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUV_RGBA)*numverts, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, true, 2, VertexFormat_XYZUV_RGBA, category, "Particles-Verts" );
}

void ParticleRendererInstanced::RebuildParticleQuad(MyMatrix* matrot)
{
    static const unsigned int numverts = 4;

    MyAssert( m_NumVertsAllocated == numverts );

    Vertex_XYZUV_RGBA* pVerts = m_pParticleQuadVerts;
    float halfsize = 0.5;
    
    //if( matrot ) // billboard the quad
    if( 0 ) // billboarding in shader so particles will work in shadowmaps.
    {
        Vector3 pos;
    
        pos.Set( -halfsize, +halfsize, 0 );
        pos = *matrot * pos;
        pVerts[0].x = pos.x; pVerts[0].y = pos.y; pVerts[0].z = pos.z; pVerts[0].u = 0; pVerts[0].v = 0;

        pos.Set( +halfsize, +halfsize, 0 );
        pos = *matrot * pos;
        pVerts[1].x = pos.x; pVerts[1].y = pos.y; pVerts[1].z = pos.z; pVerts[1].u = 1; pVerts[1].v = 0;

        pos.Set( -halfsize, -halfsize, 0 );
        pos = *matrot * pos;
        pVerts[2].x = pos.x; pVerts[2].y = pos.y; pVerts[2].z = pos.z; pVerts[2].u = 0; pVerts[2].v = 1;

        pos.Set( +halfsize, -halfsize, 0 );
        pos = *matrot * pos;
        pVerts[3].x = pos.x; pVerts[3].y = pos.y; pVerts[3].z = pos.z; pVerts[3].u = 1; pVerts[3].v = 1;
    }
    else
    {
        pVerts[0].x = -halfsize; pVerts[0].y = +halfsize; pVerts[0].z = 0; pVerts[0].u = 0; pVerts[0].v = 0;
        pVerts[1].x = +halfsize; pVerts[1].y = +halfsize; pVerts[1].z = 0; pVerts[1].u = 1; pVerts[1].v = 0;
        pVerts[2].x = -halfsize; pVerts[2].y = -halfsize; pVerts[2].z = 0; pVerts[2].u = 0; pVerts[2].v = 1;
        pVerts[3].x = +halfsize; pVerts[3].y = -halfsize; pVerts[3].z = 0; pVerts[3].u = 1; pVerts[3].v = 1;
    }

    m_pVertexBuffer->Rebuild( 0, m_pVertexBuffer->m_DataSize, true );
}

void ParticleRendererInstanced::AddPoint(Vector2 pos, float rot, ColorByte color, float size)
{
    AddPoint( Vector3( pos.x, pos.y, 0 ), rot, color, size );
}

void ParticleRendererInstanced::AddPoint(Vector3 pos, float rot, ColorByte color, float size)
{
    unsigned int vertexnum = m_ParticleCount;

    ParticleInstanceData* pVerts = (ParticleInstanceData*)m_pParticleData;

    if( vertexnum < m_NumParticlesAllocated )
    {
        pVerts[vertexnum].Set( pos.x, pos.y, pos.z, size, color.r, color.g, color.b, color.a );

        m_ParticleCount++;
    }
}

void ParticleRendererInstanced::DrawParticles(Vector3 campos, Vector3 camrot, MyMatrix* matviewproj, ShaderGroup* pShaderOverride)
{
    // Shader override is only used by mouse picker ATM, don't draw particles into mouse picker frame (or fix).
    if( pShaderOverride )
        return;

    if( m_pMaterial == 0 || m_pMaterial->GetShaderInstanced() == 0 || m_ParticleCount == 0 )
        return;

    checkGlError( "start of ParticleRenderInstanced::Draw()" );

    int numparticles = m_ParticleCount;

    if( m_pVertexBuffer->m_Dirty )
    {
        MyAssert( m_pVertexBuffer->m_DataSize != 0 );
        m_pVertexBuffer->Rebuild( 0, m_pVertexBuffer->m_DataSize );
    }
    MyAssert( m_pVertexBuffer->m_Dirty == false );

    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShaderInstanced()->GlobalPass();
    if( pShader == 0 )
        return;

    // Enable blending if necessary. TODO: sort draws and only set this once.
    if( m_pMaterial->IsTransparent( pShader ) )
    {
        glEnable( GL_BLEND );
        if( m_Additive )
        {
            glBlendFunc( GL_ONE, GL_ONE );
#if USE_D3D
            float blendfactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            g_pD3DContext->OMSetBlendState( g_pD3DBlendStateEnabledAdditive.Get(), blendfactor, 0xfff);
#endif
        }
        else
        {
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        }
    }

    if( pShader->ActivateAndProgramShader(
            m_pVertexBuffer, m_pIndexBuffer, GL_UNSIGNED_SHORT,
            matviewproj, 0, m_pMaterial ) )
    {
        GLint aiposloc = glGetAttribLocation( pShader->m_ProgramHandle, "ai_Position" );
        GLint aiscaleloc = glGetAttribLocation( pShader->m_ProgramHandle, "ai_Scale" );
        GLint aicolorloc = glGetAttribLocation( pShader->m_ProgramHandle, "ai_Color" );

        {
            glBindBuffer( GL_ARRAY_BUFFER, m_ParticleDataBuffer );
            glBufferData( GL_ARRAY_BUFFER, sizeof(ParticleInstanceData) * m_ParticleCount, m_pParticleData, GL_DYNAMIC_DRAW );

            if( aiposloc != -1 )
            {
                glVertexAttribPointer( aiposloc, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)0 );
                glEnableVertexAttribArray( aiposloc );
                glVertexAttribDivisor( aiposloc, 1 );
            }

            if( aiscaleloc != -1 )
            {
                glVertexAttribPointer( aiscaleloc, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)12 );
                glEnableVertexAttribArray( aiscaleloc );
                glVertexAttribDivisor( aiscaleloc, 1 );
            }

            if( aicolorloc != -1 )
            {
                glVertexAttribPointer( aicolorloc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ParticleInstanceData), (void*)16 );
                glEnableVertexAttribArray( aicolorloc );
                glVertexAttribDivisor( aicolorloc, 1 );
            }
        }

        pShader->ProgramCamera( 0, &camrot, 0 );

        checkGlError( "before glDrawArraysInstanced() in ParticleRenderInstanced::Draw()" );

        glDrawArraysInstanced( GL_TRIANGLE_STRIP, 0, 4, m_ParticleCount );
        pShader->DeactivateShader( m_pVertexBuffer, true );

        checkGlError( "after glDrawArraysInstanced() in ParticleRenderInstanced::Draw()" );

        if( aiposloc != -1 )
            glVertexAttribDivisor( aiposloc, 0 );
        if( aiscaleloc != -1 )
            glVertexAttribDivisor( aiscaleloc, 0 );
        if( aicolorloc != -1 )
            glVertexAttribDivisor( aicolorloc, 0 );

        if( aiposloc != -1 )
            glDisableVertexAttribArray( aiposloc );
        if( aiscaleloc != -1 )
            glDisableVertexAttribArray( aiscaleloc );
        if( aicolorloc != -1 )
            glDisableVertexAttribArray( aicolorloc );

        checkGlError( "after glVertexAttribDivisor() in ParticleRenderInstanced::Draw()" );
    }

    // always disable blending
    glDisable( GL_BLEND );

    checkGlError( "after glDisable( GL_BLEND ) in ParticleRenderInstanced::Draw()" );

    //glEnable( GL_BLEND );
    if( m_Additive ) // revert back to regular enabled alpha blending.
    {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#if USE_D3D
        float blendfactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        g_pD3DContext->OMSetBlendState( g_pD3DBlendStateEnabled.Get(), blendfactor, 0xfff);
#endif
    }

    checkGlError( "end of ParticleRenderInstanced::Draw()" );

    return;
}

#endif //MYFW_USEINSTANCEDPARTICLES
