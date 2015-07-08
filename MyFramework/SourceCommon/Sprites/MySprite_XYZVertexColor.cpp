//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MySprite_XYZVertexColor.h"

MySprite_XYZVertexColor::MySprite_XYZVertexColor(bool creatematerial)
: MySprite( creatematerial )
{
}

MySprite_XYZVertexColor::~MySprite_XYZVertexColor()
{
}

void MySprite_XYZVertexColor::Create(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts)
{
    CreateSubsection( category, spritew, spriteh, startu, endu, startv, endv, justificationflags, 0, 1, 0, 1, staticverts );
}

void MySprite_XYZVertexColor::Create(float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts)
{
    CreateSubsection( "Default", spritew, spriteh, startu, endu, startv, endv, justificationflags, 0, 1, 0, 1, staticverts );
}

void MySprite_XYZVertexColor::CreateSubsection(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, float spx, float epx, float spy, float epy, bool staticverts)
{
    MyAssert( m_SpriteIsStatic == false );

    m_SpriteSize.Set( spritew, spriteh );

    if( m_pVertexBuffer == 0 )
    {
        m_SpriteIsStatic = staticverts;

        Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[4];
        if( staticverts )
            m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_XYZUV_RGBA), GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUV_RGBA, category, "MySprite_XYZVertexColor-StaticVerts" );
        else
            m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_XYZUV_RGBA), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, false, 2, VertexFormat_XYZUV_RGBA, category, "MySprite_XYZVertexColor-Verts" );

        //m_pVAO = g_pBufferManager->CreateVAO();
    }

    if( m_pIndexBuffer == 0 )
    {
        GLushort* pIndices = MyNew GLushort[6];

        pIndices[0] = 0; // TL
        pIndices[1] = 2; // BL
        pIndices[2] = 1; // TR
        pIndices[3] = 2; // BL
        pIndices[4] = 3; // BR
        pIndices[5] = 1; // TR

        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, 6*sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, VertexFormat_None, category, "MySprite_XYZVertexColor-Indices" );
    }

    // fill vertex buffer with data and mark it dirty.
    MyAssert( m_pVertexBuffer && m_pVertexBuffer->m_pData );
    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->m_pData;
    m_pVertexBuffer->m_Dirty = true;

    for( int i=0; i<4; i++ )
    {
        pVerts[i].z = 0;
        pVerts[i].r = 255;
        pVerts[i].g = 255;
        pVerts[i].b = 255;
        pVerts[i].a = 255;
    }

    {
        float uwidth = endu - startu;
        float vheight = endv - startv;

        float uleft = startu + uwidth * spx;
        float uright = endu - uwidth * (1 - epx);
        float vtop = startv + vheight * spy;
        float vbottom = endv - vheight * (1 - epy);

        float xleft = 0;
        float xright = spritew;
        float ytop = 0;
        float ybottom = -spriteh;

        if( justificationflags & Justify_Bottom )
        {
            ytop += spriteh;
            ybottom += spriteh;
        }
        else if( justificationflags & Justify_CenterY )
        {
            ytop += spriteh / 2.0f;
            ybottom += spriteh / 2.0f;
        }

        if( justificationflags & Justify_Right )
        {
            xleft -= spritew;
            xright -= spritew;
        }
        else if( justificationflags & Justify_CenterX )
        {
            xleft -= spritew / 2.0f;
            xright -= spritew / 2.0f;
        }

        // crop down the sprite left/right/top/bottom to sizes asked for with spx/epx and spy/epy
        {
            xleft += spx*spritew;
            xright -= (1-epx)*spritew;
            ytop -= spy*spriteh;
            ybottom += (1-epy)*spriteh;
        }

        // upper left
        pVerts[0].x = xleft;
        pVerts[0].y = ytop;
        pVerts[0].u = uleft;
        pVerts[0].v = vtop;

        // upper right
        pVerts[1].x = xright;
        pVerts[1].y = ytop;
        pVerts[1].u = uright;
        pVerts[1].v = vtop;

        // lower left
        pVerts[2].x = xleft;
        pVerts[2].y = ybottom;
        pVerts[2].u = uleft;
        pVerts[2].v = vbottom;

        // lower right
        pVerts[3].x = xright;
        pVerts[3].y = ybottom;
        pVerts[3].u = uright;
        pVerts[3].v = vbottom;
    }
}

void MySprite_XYZVertexColor::FlipX()
{
    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->m_pData;

    float oldustart = pVerts[0].u;
    float olduend = pVerts[1].u;

    pVerts[0].u = olduend;
    pVerts[1].u = oldustart;
    pVerts[2].u = olduend;
    pVerts[3].u = oldustart;

    m_pVertexBuffer->m_Dirty = true;
}

void MySprite_XYZVertexColor::Draw(MyMatrix* matviewproj)
{
    if( m_pMaterial == 0 )
        return;

    MyAssert( m_pVertexBuffer != 0 && m_pIndexBuffer != 0 );

    if( m_pVertexBuffer->m_Dirty )
        m_pVertexBuffer->Rebuild( 0, m_pVertexBuffer->m_DataSize );
    if( m_pIndexBuffer->m_Dirty )
        m_pIndexBuffer->Rebuild( 0, m_pIndexBuffer->m_DataSize );
    MyAssert( m_pIndexBuffer->m_Dirty == false && m_pVertexBuffer->m_Dirty == false );

    TextureDefinition* pTexture = GetTexture();

    if( pTexture == 0 )
        return;

    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();
    if( pShader == 0 )
        return;

    // Enable blending if necessary. TODO: sort draws and only set this once.
    if( m_pMaterial->IsTransparent( pShader ) )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    if( pShader->ActivateAndProgramShader(
            m_pVertexBuffer, m_pIndexBuffer, GL_UNSIGNED_SHORT,
            matviewproj, &m_Position, m_pMaterial ) )
    {
#if USE_D3D
        g_pD3DContext->DrawIndexed( 6, 0, 0 );
#else
        MyDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );
        //LOGInfo( LOGTag, "Rendering: vbo(%d) ibo(%d)\n", m_pVertexBuffer->m_DataSize, m_pIndexBuffer->m_DataSize );
#endif
        m_pMaterial->GetShader()->GlobalPass()->DeactivateShader( m_pVertexBuffer );
    }

    // always disable blending
    glDisable( GL_BLEND );
}

void MySprite_XYZVertexColor::SetVertexColors( ColorByte bl, ColorByte br, ColorByte tl, ColorByte tr )
{
    MyAssert( m_pVertexBuffer->m_pData );

    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->m_pData;
    m_pVertexBuffer->m_Dirty = true;

    pVerts[0].r = tl.r;
    pVerts[0].g = tl.g;
    pVerts[0].b = tl.b;
    pVerts[0].a = tl.a;

    pVerts[1].r = tr.r;
    pVerts[1].g = tr.g;
    pVerts[1].b = tr.b;
    pVerts[1].a = tr.a;

    pVerts[2].r = bl.r;
    pVerts[2].g = bl.g;
    pVerts[2].b = bl.b;
    pVerts[2].a = bl.a;

    pVerts[3].r = br.r;
    pVerts[3].g = br.g;
    pVerts[3].b = br.b;
    pVerts[3].a = br.a;
}

void MySprite_XYZVertexColor::SetVertexColors( ColorFloat bl, ColorFloat br, ColorFloat tl, ColorFloat tr )
{
    MyAssert( m_pVertexBuffer->m_pData );

    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->m_pData;
    m_pVertexBuffer->m_Dirty = true;

    pVerts[0].r = (unsigned char)(tl.r * 255.0f);
    pVerts[0].g = (unsigned char)(tl.g * 255.0f);
    pVerts[0].b = (unsigned char)(tl.b * 255.0f);
    pVerts[0].a = (unsigned char)(tl.a * 255.0f);

    pVerts[1].r = (unsigned char)(tr.r * 255.0f);
    pVerts[1].g = (unsigned char)(tr.g * 255.0f);
    pVerts[1].b = (unsigned char)(tr.b * 255.0f);
    pVerts[1].a = (unsigned char)(tr.a * 255.0f);

    pVerts[2].r = (unsigned char)(bl.r * 255.0f);
    pVerts[2].g = (unsigned char)(bl.g * 255.0f);
    pVerts[2].b = (unsigned char)(bl.b * 255.0f);
    pVerts[2].a = (unsigned char)(bl.a * 255.0f);

    pVerts[3].r = (unsigned char)(br.r * 255.0f);
    pVerts[3].g = (unsigned char)(br.g * 255.0f);
    pVerts[3].b = (unsigned char)(br.b * 255.0f);
    pVerts[3].a = (unsigned char)(br.a * 255.0f);
}
