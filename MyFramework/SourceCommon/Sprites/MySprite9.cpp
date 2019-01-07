//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "../Renderers/BaseClasses/Renderer_Base.h"
#include "../Renderers/BaseClasses/Shader_Base.h"
#include "MySprite9.h"

// TODO: Fix GL Includes.
#include <gl/GL.h>
#include "../../GLExtensions.h"
#include "../Renderers/OpenGL/GLHelpers.h"

MySprite9::MySprite9()
{
    m_pMaterial = 0;

    m_pVertexBuffer = 0;
    m_pIndexBuffer = 0;

    m_Position.SetIdentity();

    m_pParentMatrix = 0;
}

MySprite9::~MySprite9()
{
    SAFE_RELEASE( m_pVertexBuffer );
    SAFE_RELEASE( m_pIndexBuffer );

    SAFE_RELEASE( m_pMaterial );
}

void MySprite9::SetMaterial(MaterialDefinition* pMaterial)
{
    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;
}

MyMatrix MySprite9::GetPosition()
{
    MyMatrix pos = m_Position;
    if( m_pParentMatrix )
        pos = *m_pParentMatrix * pos;

    return pos;
}

void MySprite9::SetPosition(Vector3 pos, bool setindentity)
{
    if( setindentity )
        m_Position.SetIdentity();
    m_Position.SetTranslation( pos );
}

void MySprite9::SetPosition(float x, float y, float z, bool setindentity)
{
    if( setindentity )
        m_Position.SetIdentity();
    m_Position.SetTranslation( x, y, z );
}

void MySprite9::SetPosition(MyMatrix* mat)
{
    m_Position = *mat;
}

void MySprite9::Create(float x1, float x2, float x3, float x4, float y1, float y2, float y3, float y4, float u1, float u2, float u3, float u4, float v1, float v2, float v3, float v4)
{
    // allocate 16 verts
    if( m_pVertexBuffer == 0 )
    {
        Vertex_Sprite* pVerts = MyNew Vertex_Sprite[16];
        m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 16*sizeof(Vertex_Sprite), MyRE::BufferType_Vertex, MyRE::BufferUsage_DynamicDraw, false, 2, VertexFormat_Sprite, "MySprite9", "Verts" );
    }

    // allocate and fill 24 indices for a triangle strip
    if( m_pIndexBuffer == 0 )
    {
        //  0  1  2  3
        //  4  5  6  7
        //  8  9 10 11
        // 12 13 14 15

        // 0  4  1  5  2  6  3  7 *7* 11  6 10  5  9  4  8 *8* 12  9 13 10 14 11 15
        // / +4 -3 +4 -3 +4 -3 +4  /  +4 -5 +4 -5 +4 -5 +4  /  +4 -3 +4 -3 +4 -3 +4

        GLushort cols = 4;
        GLushort rows = 4;
        int vertsperrow = cols*2;
        int numindices = vertsperrow * (rows-1);

        GLushort* pIndices = MyNew GLushort[numindices];

        GLushort vertindex = 0;
        for( GLushort i=0; i<numindices; i++ )
        {
            if( i % vertsperrow == 0 )
                vertindex += 0;
            else
            {
                if( (i/vertsperrow) % 2 == 0 )
                    vertindex += ( i % 2 == 1 ? cols : -cols+1 ); // going right
                else
                    vertindex += ( i % 2 == 1 ? cols : -cols-1 ); // going left
            }

            pIndices[i] = vertindex;
        }
        
        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, numindices*sizeof(GLushort), MyRE::BufferType_Index, MyRE::BufferUsage_StaticDraw, true, 1, VertexFormat_None, "MySprite9", "Verts" );
    }

    // Fill vertex buffer with data and mark it dirty.
    {
        MyAssert( m_pVertexBuffer && m_pVertexBuffer->GetData( false ) );
        Vertex_Sprite* pVerts = (Vertex_Sprite*)m_pVertexBuffer->GetData( true );

        for( int y=0; y<4; y++ )
        {
            pVerts[y*4+0].x = x1;
            pVerts[y*4+1].x = x2;
            pVerts[y*4+2].x = x3;
            pVerts[y*4+3].x = x4;

            pVerts[y*4+0].u = u1;
            pVerts[y*4+1].u = u2;
            pVerts[y*4+2].u = u3;
            pVerts[y*4+3].u = u4;
        }

        for( int x=0; x<4; x++ )
        {
            pVerts[4*0+x].y = y1;
            pVerts[4*1+x].y = y2;
            pVerts[4*2+x].y = y3;
            pVerts[4*3+x].y = y4;

            pVerts[4*0+x].v = v1;
            pVerts[4*1+x].v = v2;
            pVerts[4*2+x].v = v3;
            pVerts[4*3+x].v = v4;
        }
    }
}

void MySprite9::SetTint(ColorByte tintcolor)
{
    m_pMaterial->m_ColorDiffuse = tintcolor;
}

void MySprite9::Draw(MyMatrix* pMatProj, MyMatrix* pMatView)
{
    if( m_pMaterial == 0 || m_pMaterial->GetShader() == 0 )

    MyAssert( m_pVertexBuffer != 0 && m_pIndexBuffer != 0 );

    if( m_pVertexBuffer->IsDirty() )
    {
        m_pVertexBuffer->Rebuild();
    }
    if( m_pIndexBuffer->IsDirty() )
    {
        m_pIndexBuffer->Rebuild();
    }
    MyAssert( m_pVertexBuffer->IsDirty() == false && m_pIndexBuffer->IsDirty() == false );

    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();
    if( pShader == 0 )
        return;

    // Enable blending if necessary. TODO: sort draws and only set this once.
    if( m_pMaterial->IsTransparent( pShader ) )
    {
        g_pRenderer->SetBlendEnabled( true );
        g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );
    }

    if( pShader->ActivateAndProgramShader(
            m_pVertexBuffer, m_pIndexBuffer, MyRE::IndexType_U16, 
            pMatProj, pMatView, &m_Position, m_pMaterial ) )
    {
#if USE_D3D
        g_pD3DContext->DrawIndexed( 6, 0, 0 );
        //g_pD3DContext->Draw( 6, 0 );
#else
        g_pRenderer->DrawElements( MyRE::PrimitiveType_TriangleStrip, 24, MyRE::IndexType_U16, 0, false );
#endif
        pShader->DeactivateShader( m_pVertexBuffer, true );
    }

    // Always disable blending.
    g_pRenderer->SetBlendEnabled( false );
}
