//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MySprite_XYZVertexColor.h"
#include "../DataTypes/ColorStructs.h"
#include "../Meshes/BufferManager.h"
#include "../Renderers/BaseClasses/Renderer_Base.h"
#include "../Renderers/BaseClasses/Shader_Base.h"
#include "../Textures/MaterialDefinition.h"

MySprite_XYZVertexColor::MySprite_XYZVertexColor()
: MySprite()
{
}

MySprite_XYZVertexColor::~MySprite_XYZVertexColor()
{
}

void MySprite_XYZVertexColor::Create(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts, bool facepositivez)
{
    CreateSubsection( category, spritew, spriteh, startu, endu, startv, endv, justificationflags, 0, 1, 0, 1, staticverts, facepositivez );
}

void MySprite_XYZVertexColor::Create(float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts, bool facepositivez)
{
    CreateSubsection( "Default", spritew, spriteh, startu, endu, startv, endv, justificationflags, 0, 1, 0, 1, staticverts, facepositivez );
}

void MySprite_XYZVertexColor::CreateSubsection(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, float spx, float epx, float spy, float epy, bool staticverts, bool facepositivez)
{
    MyAssert( m_SpriteIsStatic == false );

    m_SpriteSize.Set( spritew, spriteh );

    if( m_pVertexBuffer == 0 )
    {
        m_SpriteIsStatic = staticverts;

        Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[4];
        if( staticverts )
            m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_XYZUV_RGBA), MyRE::BufferType_Vertex, MyRE::BufferUsage_StaticDraw, false, 1, VertexFormat_XYZUV_RGBA, category, "MySprite_XYZVertexColor-StaticVerts" );
        else
            m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_XYZUV_RGBA), MyRE::BufferType_Vertex, MyRE::BufferUsage_DynamicDraw, false, 2, VertexFormat_XYZUV_RGBA, category, "MySprite_XYZVertexColor-Verts" );
    }

    if( m_pIndexBuffer == 0 )
    {
        GLushort* pIndices = MyNew GLushort[6];

        pIndices[0] = g_SpriteVertexIndices[0];
        pIndices[1] = g_SpriteVertexIndices[1];
        pIndices[2] = g_SpriteVertexIndices[2];
        pIndices[3] = g_SpriteVertexIndices[3];
        pIndices[4] = g_SpriteVertexIndices[4];
        pIndices[5] = g_SpriteVertexIndices[5];

        m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, 6*sizeof(GLushort), MyRE::BufferType_Index, MyRE::BufferUsage_StaticDraw, true, 1, VertexFormat_None, category, "MySprite_XYZVertexColor-Indices" );
    }

    // Fill vertex buffer with data and mark it dirty.
    MyAssert( m_pVertexBuffer && m_pVertexBuffer->GetData( false ) );
    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->GetData( true );

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

        // Crop down the sprite left/right/top/bottom to sizes asked for with spx/epx and spy/epy.
        {
            xleft += spx*spritew;
            xright -= (1-epx)*spritew;
            ytop -= spy*spriteh;
            ybottom += (1-epy)*spriteh;
        }

        // Upper left.
        pVerts[0].x = xleft;
        pVerts[0].y = ytop;
        pVerts[0].u = uleft;
        pVerts[0].v = vtop;

        // Upper right.
        pVerts[1].x = xright;
        pVerts[1].y = ytop;
        pVerts[1].u = uright;
        pVerts[1].v = vtop;

        // Lower left.
        pVerts[2].x = xleft;
        pVerts[2].y = ybottom;
        pVerts[2].u = uleft;
        pVerts[2].v = vbottom;

        // Lower right.
        pVerts[3].x = xright;
        pVerts[3].y = ybottom;
        pVerts[3].u = uright;
        pVerts[3].v = vbottom;

        if( facepositivez )
        {
            pVerts[0].x = xright;
            pVerts[1].x = xleft;
            pVerts[2].x = xright;
            pVerts[3].x = xleft;
        }
    }
}

void MySprite_XYZVertexColor::FlipX()
{
    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->GetData( true );

    float oldustart = pVerts[0].u;
    float olduend = pVerts[1].u;

    pVerts[0].u = olduend;
    pVerts[1].u = oldustart;
    pVerts[2].u = olduend;
    pVerts[3].u = oldustart;
}

void MySprite_XYZVertexColor::Draw(MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, ShaderGroup* pShaderOverride, bool hideFromDrawList)
{
    Draw( nullptr, nullptr, pMatProj, pMatView, pMatWorld, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, pShaderOverride, hideFromDrawList );
}

void MySprite_XYZVertexColor::Draw(MaterialDefinition* pMaterial, MyMesh* pMesh, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, Vector3* campos, Vector3* camrot, MyLight** lightptrs, int numlights, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride, bool hideFromDrawList)
{
    if( pMaterial == nullptr )
        pMaterial = m_pMaterial;

    MyAssert( pShaderOverride == nullptr ); // TODO: Support overriding shaders

    if( pMaterial == nullptr )
        return;

    MyAssert( m_pVertexBuffer != nullptr && m_pIndexBuffer != nullptr );

    if( m_pVertexBuffer->IsDirty() )
    {
        m_pVertexBuffer->Rebuild();
    }
    if( m_pIndexBuffer->IsDirty() )
    {
        m_pIndexBuffer->Rebuild();
    }
    MyAssert( m_pVertexBuffer->IsDirty() == false && m_pIndexBuffer->IsDirty() == false );

    TextureDefinition* pTexture = GetTexture();

    if( pTexture == 0 )
        return;

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
            pMatProj, pMatView, pMatWorld, m_pMaterial ) )
    {
#if USE_D3D
        g_pD3DContext->DrawIndexed( 6, 0, 0 );
#else
        g_pRenderer->DrawElements( MyRE::PrimitiveType_Triangles, 6, MyRE::IndexType_U16, 0, hideFromDrawList );
        //LOGInfo( LOGTag, "Rendering: vbo(%d) ibo(%d)\n", m_pVertexBuffer->m_DataSize, m_pIndexBuffer->m_DataSize );
#endif
        m_pMaterial->GetShader()->GlobalPass()->DeactivateShader( m_pVertexBuffer );
    }

    // Always disable blending.
    g_pRenderer->SetBlendEnabled( false );
}

void MySprite_XYZVertexColor::SetVertexColors( ColorByte bl, ColorByte br, ColorByte tl, ColorByte tr )
{
    MyAssert( m_pVertexBuffer->GetData( false ) );

    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->GetData( true );

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
    MyAssert( m_pVertexBuffer->GetData( false ) );

    Vertex_XYZUV_RGBA* pVerts = (Vertex_XYZUV_RGBA*)m_pVertexBuffer->GetData( true );

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
