//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MySprite.h"

// These are 2 sets of indices for sprites, one winding clockwise, the other counter clockwise.
//   for code to use them, you'd need to fill your vertex buffer in this order.
// 0--1
// |\ |
// | \|
// 3--2
#if MYFW_RIGHTHANDED
//                                TL, BL, TR,   BL, BR, TR   
char g_SpriteVertexIndices[6] = {  0,  2,  1,    2,  3,  1 }; // counter-clockwise winding
#else                                                           
//                                TL, TR, BL,   BL, TR, BR
char g_SpriteVertexIndices[6] = {  0,  1,  2,    2,  1,  3 }; // clockwise winding
#endif

MySprite::MySprite(bool creatematerial)
: m_Tint(255, 255, 255, 255)
{
    m_SpriteIsStatic = false;

    m_pMaterial = 0;

    if( creatematerial )
    {
        m_pMaterial = g_pMaterialManager->CreateMaterial();
        m_pMaterial->SetShader( g_pShaderGroupManager->FindShaderGroupByName( "Shader_TintColor" ) );
    }

    m_pVertexBuffer = 0;
    m_pIndexBuffer = 0;
    m_pVAO = 0;

    m_Position.SetIdentity();

    m_pParentMatrix = 0;

    m_SpriteSize.Set( 0, 0 );
    m_SpriteUVStart.Set( 0, 0 );
    m_SpriteUVEnd.Set( 1, 1 );
    m_SpriteJustification = Justify_Center;
}

MySprite::MySprite(MySprite* pSprite, const char* category)
{
    MyAssert( pSprite != 0 );

    *this = *pSprite;

    m_pMaterial = g_pMaterialManager->CreateMaterial();
    m_pMaterial->SetShader( g_pShaderGroupManager->FindShaderGroupByName( "Shader_TintColor" ) );

    Vertex_Sprite* pVerts = MyNew Vertex_Sprite[4];
    memcpy( pVerts, pSprite->m_pVertexBuffer->m_pData, sizeof(Vertex_Sprite)*4);
    m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_Sprite), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, false, 2, VertexFormat_Sprite, category, "MySprite-Verts" );
    //m_pVAO = g_pBufferManager->CreateVAO();

    m_pIndexBuffer->AddRef();
}

MySprite::~MySprite()
{
    SAFE_RELEASE( m_pVertexBuffer );
    SAFE_RELEASE( m_pIndexBuffer );

    SAFE_RELEASE( m_pMaterial );
    //SAFE_RELEASE( m_pVAO );
}

void MySprite::Create(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts, bool facepositivez)
{
    CreateSubsection( category, spritew, spriteh, startu, endu, startv, endv, justificationflags, 0, 1, 0, 1, staticverts, facepositivez );
}

void MySprite::Create(float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts, bool facepositivez)
{
    CreateSubsection( "MySprite", spritew, spriteh, startu, endu, startv, endv, justificationflags, 0, 1, 0, 1, staticverts, facepositivez );
}

void MySprite::CreateSubsection(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, float spx, float epx, float spy, float epy, bool staticverts, bool facepositivez)
{
    MyAssert( m_SpriteIsStatic == false );

    if( m_SpriteSize.x != spritew || m_SpriteSize.y != spriteh ||
        m_SpriteUVStart.x != startu || m_SpriteUVStart.y != startv ||
        m_SpriteUVEnd.x != endu || m_SpriteUVEnd.y != endv ||
        m_SpriteJustification != justificationflags )
    {
        if( spx != 0 || spy != 0 || epx != 1 || epy != 1 )
            MyAssert( false ); // subsections not implemented.

        if( m_pVertexBuffer == 0 )
        {
            m_SpriteIsStatic = staticverts;

            Vertex_Sprite* pVerts = MyNew Vertex_Sprite[4];
            if( staticverts )
                m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_Sprite), GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_Sprite, category, "MySprite-Static Verts" );
            else
                m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_Sprite), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, false, 2, VertexFormat_Sprite, category, "MySprite-Verts" );

            //m_pVAO = g_pBufferManager->CreateVAO();
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

            m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, 6*sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, VertexFormat_None, category, "MySprite-Indices" );
        }

        // fill vertex buffer with data and mark it dirty.
        {
            MyAssert( m_pVertexBuffer && m_pVertexBuffer->m_pData );
            Vertex_Sprite* pVerts = (Vertex_Sprite*)m_pVertexBuffer->m_pData;

            float uleft = startu;
            float uright = endu;
            float vtop = startv;
            float vbottom = endv;

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

            if( facepositivez == true )
            {
                pVerts[0].x = xright;
                pVerts[1].x = xleft;
                pVerts[2].x = xright;
                pVerts[3].x = xleft;
            }

            m_pVertexBuffer->m_Dirty = true;
        }

        m_SpriteSize.Set( spritew, spriteh );
        m_SpriteUVStart.Set( startu, startv );
        m_SpriteUVEnd.Set( endu, endv );
        m_SpriteJustification = justificationflags;
    }
    else
    {
        //int bp = 1;
    }
}

void MySprite::CreateInPlace(const char* category, float x, float y, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts, bool facepositivez)
{
    MyAssert( m_SpriteIsStatic == false );

    if( m_SpriteSize.x != spritew || m_SpriteSize.y != spriteh ||
        m_SpriteUVStart.x != startu || m_SpriteUVStart.y != startv ||
        m_SpriteUVEnd.x != endu || m_SpriteUVEnd.y != endv )
    {
        if( m_pVertexBuffer == 0 )
        {
            m_SpriteIsStatic = staticverts;

            Vertex_Sprite* pVerts = MyNew Vertex_Sprite[4];
            if( staticverts )
                m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_Sprite), GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_Sprite, category, "MySprite-Static Verts" );
            else
                m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, 4*sizeof(Vertex_Sprite), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, false, 2, VertexFormat_Sprite, category, "MySprite-Verts" );
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

            m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, 6*sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, VertexFormat_None, category, "MySprite-Indices" );
        }

        // fill vertex buffer with data and mark it dirty.
        {
            MyAssert( m_pVertexBuffer && m_pVertexBuffer->m_pData );
            Vertex_Sprite* pVerts = (Vertex_Sprite*)m_pVertexBuffer->m_pData;

            float uleft = startu;
            float uright = endu;
            float vtop = startv;
            float vbottom = endv;

            float xleft = x;
            float xright = x + spritew;
            float ytop = y;
            float ybottom = y - spriteh;

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

            m_pVertexBuffer->m_Dirty = true;
        }

        m_SpriteSize.Set( spritew, spriteh );
        m_SpriteUVStart.Set( startu, startv );
        m_SpriteUVEnd.Set( endu, endv );
        m_SpriteJustification = Justify_Center;
    }
    else
    {
        //int bp = 1;
    }
}

MyMatrix MySprite::GetPosition()
{
    MyMatrix pos = m_Position;
    if( m_pParentMatrix )
        pos = *m_pParentMatrix * pos;

    return pos;
}

void MySprite::SetPosition(Vector3 pos, bool setindentity)
{
    if( setindentity )
        m_Position.SetIdentity();
    m_Position.SetTranslation( pos );
}

void MySprite::SetPosition(float x, float y, float z, bool setindentity)
{
    if( setindentity )
        m_Position.SetIdentity();
    m_Position.SetTranslation( x, y, z );
}

void MySprite::SetPosition(MyMatrix* mat)
{
    m_Position = *mat;
}

void MySprite::SetZRotation(float rotation)
{
    SetZRotationWithPivot( rotation );
}

void MySprite::SetZRotationWithPivot(float rotation, bool preserveposition, Vector3* localpivot)
{
    Vector3 oldpos;
    if( preserveposition )
        oldpos = Vector3( m_Position.m41, m_Position.m42, m_Position.m43 );

    m_Position.SetIdentity();
    if( localpivot )
        m_Position.SetTranslation( *localpivot * -1 );
    m_Position.Rotate( rotation, 0, 0, 1 );
    if( localpivot )
        m_Position.Translate( *localpivot );

    if( preserveposition )
        m_Position.Translate( oldpos );
}

void MySprite::SetRST(Vector3 rot, Vector3 scale, Vector3 pos, bool setindentity)
{
    if( setindentity )
        m_Position.SetIdentity();
    m_Position.Rotate( rot.x, 1, 0, 0 );
    m_Position.Rotate( rot.y, 0, 1, 0 );
    m_Position.Rotate( rot.z, 0, 0, 1 );
    m_Position.Scale( scale.x, scale.y, scale.z );
    m_Position.Translate( pos.x, pos.y, pos.z );
}

void MySprite::SetTransform(MyMatrix& mat)
{
    m_Position = mat;
}

void MySprite::SetTint(ColorByte tintcolor)
{
    //LOGError( LOGTag, "SetTint is obsolete, use materials.\n" );
    m_Tint = tintcolor;
}

void MySprite::FlipX()
{
    Vertex_Sprite* pVerts = (Vertex_Sprite*)m_pVertexBuffer->m_pData;

    float oldustart = pVerts[0].u;
    float olduend = pVerts[1].u;

    pVerts[0].u = olduend;
    pVerts[1].u = oldustart;
    pVerts[2].u = olduend;
    pVerts[3].u = oldustart;

    m_pVertexBuffer->m_Dirty = true;
}

void MySprite::SetMaterial(MaterialDefinition* pMaterial)
{
    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;

    // rebuild the vaos in case the attributes required for the shader are different than the last shader assigned.
    if( m_pVertexBuffer )
        m_pVertexBuffer->ResetVAOs();
}

bool MySprite::Setup(MyMatrix* matviewproj)
{
    if( m_pMaterial == 0 )
        return false;

    MyAssert( m_pVertexBuffer != 0 && m_pIndexBuffer != 0 );

    if( m_pVertexBuffer->m_Dirty )
        m_pVertexBuffer->Rebuild( 0, m_pVertexBuffer->m_DataSize );
    if( m_pIndexBuffer->m_Dirty )
        m_pIndexBuffer->Rebuild( 0, m_pIndexBuffer->m_DataSize );
    MyAssert( m_pIndexBuffer->m_Dirty == false && m_pVertexBuffer->m_Dirty == false );

    //TextureDefinition* pTexture = GetTexture();

    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();
    if( pShader == 0 )
        return false;

    // Enable blending if necessary. TODO: sort draws and only set this once.
    if( m_pMaterial->IsTransparent( pShader ) )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    bool activated = pShader->ActivateAndProgramShader(
                        m_pVertexBuffer, m_pIndexBuffer, GL_UNSIGNED_SHORT,
                        matviewproj, &m_Position, m_pMaterial );

    // always disable blending
    glDisable( GL_BLEND );

    return activated;
}

void MySprite::DrawNoSetup()
{
    if( m_pMaterial == 0 )
        return;

#if USE_D3D
    g_pD3DContext->DrawIndexed( 6, 0, 0 );
    //g_pD3DContext->Draw( 6, 0 );
#else
    MyDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );
#endif
}

void MySprite::DeactivateShader()
{
    if( m_pMaterial == 0 )
        return;

    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();
    if( pShader == 0 )
        return;

    pShader->DeactivateShader( m_pVertexBuffer );
}

void MySprite::Draw(MyMatrix* matviewproj, ShaderGroup* pShaderOverride)
{
    if( m_pMaterial == 0 || m_pMaterial->GetShader() == 0 )
        return;

    MyAssert( m_pVertexBuffer != 0 && m_pIndexBuffer != 0 );

    if( m_pVertexBuffer->m_Dirty )
        m_pVertexBuffer->Rebuild( 0, m_pVertexBuffer->m_DataSize );
    if( m_pIndexBuffer->m_Dirty )
        m_pIndexBuffer->Rebuild( 0, m_pIndexBuffer->m_DataSize );
    MyAssert( m_pIndexBuffer->m_Dirty == false && m_pVertexBuffer->m_Dirty == false );

    Shader_Base* pShader = 0;
    if( pShaderOverride )
    {
        // if an override for the shader is sent in, it's already active and doesn't want anything other than position set.
        pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );

        MyAssert( pShader );
        if( pShader == 0 )
            return;

        pShader->SetupAttributes( m_pVertexBuffer, m_pIndexBuffer, false );
        pShader->ProgramPosition( matviewproj, &m_Position );

        MyDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );
        pShader->DeactivateShader( m_pVertexBuffer );
    }
    else
    {
        pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass();

        MyAssert( pShader );
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
            pShader->ProgramFramebufferSize( (float)g_GLStats.m_CurrentFramebufferWidth, (float)g_GLStats.m_CurrentFramebufferHeight );

            MyDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );
            pShader->DeactivateShader( m_pVertexBuffer );
        }

        // always disable blending
        glDisable( GL_BLEND );
    }
}

Vertex_Base* MySprite::GetVerts(bool markdirty)
{
    MyAssert( m_pVertexBuffer );

    if( markdirty )
        m_pVertexBuffer->m_Dirty = true;

    return (Vertex_Base*)m_pVertexBuffer->m_pData;
}

TextureDefinition* MySprite::GetTexture()
{
    if( m_pMaterial == 0 )
        return 0;

    return m_pMaterial->GetTextureColor();
}
