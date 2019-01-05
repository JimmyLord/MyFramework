//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "../Renderers/BaseClasses/Renderer_Base.h"
#include "../Renderers/BaseClasses/Shader_Base.h"

#include "Box2DDebugDraw.h"

// TODO: Fix GL Includes.
#include <gl/GL.h>
#include "../../GLExtensions.h"
#include "../Renderers/OpenGL/GLHelpers.h"
#include "../Renderers/OpenGL/Shader_OpenGL.h"

Box2DDebugDraw::Box2DDebugDraw(MaterialDefinition* debugdrawmaterial, MyMatrix* pMatProj, MyMatrix* pMatView)
{
    m_pMatProj = pMatProj;
    m_pMatView = pMatView;

    m_pMaterial = debugdrawmaterial;
    if( m_pMaterial )
    {
        m_pMaterial->AddRef();
    }
}

Box2DDebugDraw::~Box2DDebugDraw()
{
    SAFE_RELEASE( m_pMaterial );
}

void Box2DDebugDraw::Draw(const b2Vec2* vertices, int32 vertexCount, const b2Color& color, unsigned char alpha, MyRE::PrimitiveTypes primitiveType, float pointorlinesize)
{
    // Set the material to the correct color and draw the shape.
    Shader_OpenGL* pShader = (Shader_OpenGL*)m_pMaterial->GetShader()->GlobalPass( 0, 0 );
    if( pShader->Activate() == false )
        return;

    m_pMaterial->SetColorDiffuse( ColorByte( (unsigned char)(color.r*255), (unsigned char)(color.g*255), (unsigned char)(color.b*255), alpha ) );

    // Setup our position attribute, pass in the array of verts, not using a VBO.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    pShader->InitializeAttributeArray( pShader->m_aHandle_Position, 2, MyRE::AttributeType_Float, false, sizeof(float)*2, (void*)vertices );

    // Setup uniforms, mainly viewproj and tint.
    pShader->ProgramMaterialProperties( 0, m_pMaterial->m_ColorDiffuse, m_pMaterial->m_ColorSpecular, m_pMaterial->m_Shininess );
    pShader->ProgramTransforms( m_pMatProj, m_pMatView, 0 );

    glLineWidth( pointorlinesize );
#ifndef MYFW_OPENGLES2
    glPointSize( pointorlinesize );
#endif

    g_pRenderer->SetBlendEnabled( true );
    g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );

    g_pRenderer->SetCullingEnabled( false );
    g_pRenderer->SetDepthTestEnabled( false );

    g_pRenderer->DrawArrays( primitiveType, 0, vertexCount, true );

    g_pRenderer->SetCullingEnabled( true );
    g_pRenderer->SetDepthTestEnabled( true );

    // Always disable blending.
    g_pRenderer->SetBlendEnabled( false );
}

void Box2DDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    Draw( vertices, vertexCount, color, 255, MyRE::PrimitiveType_LineLoop, 1 );
}

void Box2DDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    Draw( vertices, vertexCount, color, 128, MyRE::PrimitiveType_TriangleFan, 1 );
    Draw( vertices, vertexCount, color, 255, MyRE::PrimitiveType_LineLoop, 1 );
}

void Box2DDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
    static const int vertexCount = 24;
    float anglechange = 2.0f * PI / vertexCount;
	
    b2Vec2 vertices[vertexCount];
    for( int i=0; i<vertexCount; i++ )
    {
        vertices[i].x = center.x + cos( i*anglechange ) * radius;
        vertices[i].y = center.y + sin( i*anglechange ) * radius;
    }
	
    Draw( vertices, vertexCount, color, 128, MyRE::PrimitiveType_TriangleFan, 1 );
}

void Box2DDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
    static const int vertexCount = 24;
    float anglechange = 2.0f * PI / vertexCount;
	
    b2Vec2 vertices[vertexCount];
    for( int i=0; i<vertexCount; i++ )
    {
        vertices[i].x = center.x + cos( i*anglechange ) * radius;
        vertices[i].y = center.y + sin( i*anglechange ) * radius;
    }
	
    Draw( vertices, vertexCount, color, 128, MyRE::PrimitiveType_TriangleFan, 1 );
    Draw( vertices, vertexCount, color, 255, MyRE::PrimitiveType_LineLoop, 1 );
}

void Box2DDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    b2Vec2 vertices[2] = { p1, p2 };

    Draw( vertices, 2, color, 255, MyRE::PrimitiveType_Lines, 1 );
}

void Box2DDebugDraw::DrawTransform(const b2Transform& xf)
{
    b2Vec2 vertices[2] = { xf.p };

    vertices[1] = xf.p + 0.5f * xf.q.GetXAxis();
    Draw( vertices, 2, b2Color( 1, 0, 0 ), 255, MyRE::PrimitiveType_Lines, 1 );

    vertices[1] = xf.p + 0.5f * xf.q.GetYAxis();
    Draw( vertices, 2, b2Color( 0, 1, 0 ), 255, MyRE::PrimitiveType_Lines, 1 );
}

void Box2DDebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
    Draw( &p, 1, color, 255, MyRE::PrimitiveType_Points, size );
}
