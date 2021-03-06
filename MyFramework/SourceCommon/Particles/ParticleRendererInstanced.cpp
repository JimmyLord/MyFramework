//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "../Meshes/BufferManager.h"
#include "../Renderers/BaseClasses/Renderer_Base.h"
#include "../Textures/MaterialDefinition.h"

#if MYFW_USEINSTANCEDPARTICLES

#include "ParticleRendererInstanced.h"
#include "../Renderers/BaseClasses/Shader_Base.h"

ParticleRendererInstanced::ParticleRendererInstanced(bool creatematerial)
: ParticleRenderer( creatematerial )
{
    m_NumParticlesAllocated = 0;
    m_pParticleData = nullptr;
    m_pInstancedAttributesBuffer = nullptr;
}

ParticleRendererInstanced::~ParticleRendererInstanced()
{
    delete[] m_pParticleData;
    SAFE_RELEASE( m_pInstancedAttributesBuffer );
}

void ParticleRendererInstanced::AllocateVertices(unsigned int maxPoints, const char* category, BufferManager* pBufferManager)
{
    LOGInfo( LOGTag, "ParticleRendererInstanced: Allocating %d Verts\n", maxPoints );

    MyAssert( m_pVertexBuffer == nullptr );

    unsigned int numverts = 4;

    m_pParticleData = MyNew ParticleInstanceData[maxPoints];
    m_NumParticlesAllocated = maxPoints;

    m_pInstancedAttributesBuffer = pBufferManager->CreateBuffer();
    m_pInstancedAttributesBuffer->InitializeBuffer( nullptr, 0, MyRE::BufferType_Vertex, MyRE::BufferUsage_StreamDraw,
                                                    false, 1, VertexFormat_Dynamic, nullptr, "Particles", "InstancedAttribs" );

    Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[numverts];

    float halfsize = 0.5f;
    pVerts[0].x = -halfsize; pVerts[0].y = +halfsize; pVerts[0].z = 0; pVerts[0].u = 0; pVerts[0].v = 0;
    pVerts[1].x = +halfsize; pVerts[1].y = +halfsize; pVerts[1].z = 0; pVerts[1].u = 1; pVerts[1].v = 0;
    pVerts[2].x = -halfsize; pVerts[2].y = -halfsize; pVerts[2].z = 0; pVerts[2].u = 0; pVerts[2].v = 1;
    pVerts[3].x = +halfsize; pVerts[3].y = -halfsize; pVerts[3].z = 0; pVerts[3].u = 1; pVerts[3].v = 1;
    m_NumVertsAllocated = numverts;

    m_pVertexBuffer = pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUV_RGBA)*numverts, MyRE::BufferType_Vertex, MyRE::BufferUsage_DynamicDraw, true, 2, VertexFormat_XYZUV_RGBA, category, "Particles-Verts" );
}

void ParticleRendererInstanced::RebuildParticleQuad(MyMatrix* matrot)
{
    // Never rebuild quad since billboarding is done in shader allowing particles to work in shadowmaps.
    return;

    static const unsigned int numverts = 4;

    MyAssert( m_NumVertsAllocated == numverts );

    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->GetData( true );
    
    float halfsize = 0.5f;
    
    if( matrot ) // Billboard the quad.
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

    m_pVertexBuffer->Rebuild();
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

void ParticleRendererInstanced::DrawParticles(Vector3 campos, Vector3 camrot, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    // Shader override is only used by mouse picker ATM, don't draw particles into mouse picker frame (or fix).
    if( pShaderOverride )
        return;

    if( m_pMaterial == nullptr || m_pMaterial->GetShaderInstanced() == nullptr || m_ParticleCount == 0 )
        return;

    //checkGlError( "start of ParticleRenderInstanced::Draw()" );

    int numparticles = m_ParticleCount;

    if( m_pVertexBuffer->IsDirty() )
    {
        MyAssert( m_pVertexBuffer->GetDataSize() != 0 );
        m_pVertexBuffer->Rebuild();
    }
    MyAssert( m_pVertexBuffer->IsDirty() == false );

    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShaderInstanced()->GlobalPass();
    if( pShader == nullptr )
        return;

    // Enable blending if necessary. TODO: sort draws and only set this once.
    if( m_pMaterial->IsTransparent( pShader ) )
    {
        g_pRenderer->SetBlendEnabled( true );
        if( m_Additive )
        {
            g_pRenderer->SetBlendFunc( MyRE::BlendFactor_One, MyRE::BlendFactor_One );
#if USE_D3D
            float blendfactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            g_pD3DContext->OMSetBlendState( g_pD3DBlendStateEnabledAdditive.Get(), blendfactor, 0xfff);
#endif
        }
        else
        {
            g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );
        }
    }

    if( pShader->ActivateAndProgramShader(
            m_pVertexBuffer, m_pIndexBuffer, MyRE::IndexType_U16,
            pMatProj, pMatView, nullptr, m_pMaterial ) )
    {
        m_pInstancedAttributesBuffer->TempBufferData( sizeof(ParticleInstanceData) * m_ParticleCount, m_pParticleData );

        pShader->ProgramCamera( nullptr, &camrot );

        g_pRenderer->TempHack_SetupAndDrawInstanced( pShader, m_ParticleCount );

        pShader->DeactivateShader( m_pVertexBuffer, true );
    }

    // Always disable blending.
    g_pRenderer->SetBlendEnabled( false );

    if( m_Additive ) // Revert back to regular alpha blend func.
    {
        g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );
#if USE_D3D
        float blendfactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        g_pD3DContext->OMSetBlendState( g_pD3DBlendStateEnabled.Get(), blendfactor, 0xfff);
#endif
    }

    return;
}

#endif //MYFW_USEINSTANCEDPARTICLES
