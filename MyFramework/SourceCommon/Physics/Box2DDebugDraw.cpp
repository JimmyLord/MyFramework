//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "Box2DDebugDraw.h"

Box2DDebugDraw::Box2DDebugDraw(MaterialDefinition* debugdrawmaterial, MyMatrix* matviewproj)
{
    m_pMatViewProj = matviewproj;

    m_pMaterial = debugdrawmaterial;
    m_pMaterial->AddRef();
}

Box2DDebugDraw::~Box2DDebugDraw()
{
    SAFE_RELEASE( m_pMaterial );
}

void Box2DDebugDraw::Draw(const b2Vec2* vertices, int32 vertexCount, const b2Color& color, unsigned char alpha, int primitivetype, float pointorlinesize)
{
    // Set the material to the correct color and draw the shape.
    Shader_Base* pShader = (Shader_Base*)m_pMaterial->GetShader()->GlobalPass( 0, 0 );
    if( pShader->ActivateAndProgramShader() == false )
        return;

    m_pMaterial->SetColorDiffuse( ColorByte( (unsigned char)(color.r*255), (unsigned char)(color.g*255), (unsigned char)(color.b*255), alpha ) );

    // Setup our position attribute, pass in the array of verts, not using a VBO.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    pShader->InitializeAttributeArray( pShader->m_aHandle_Position, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)vertices );

    // Setup uniforms, mainly viewproj and tint.
    pShader->ProgramBaseUniforms( m_pMatViewProj, 0, 0, m_pMaterial->m_ColorDiffuse, m_pMaterial->m_ColorSpecular, m_pMaterial->m_Shininess );

    glLineWidth( pointorlinesize );
    glPointSize( pointorlinesize );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );

    MyDrawArrays( primitivetype, 0, vertexCount );

    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );

    glDisable( GL_BLEND );
}

void Box2DDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    Draw( vertices, vertexCount, color, 255, GL_LINE_LOOP, 1 );
}

void Box2DDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    Draw( vertices, vertexCount, color, 128, GL_TRIANGLE_FAN, 1 );
    Draw( vertices, vertexCount, color, 255, GL_LINE_LOOP, 1 );
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
	
    Draw( vertices, vertexCount, color, 128, GL_TRIANGLE_FAN, 1 );
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
	
    Draw( vertices, vertexCount, color, 128, GL_TRIANGLE_FAN, 1 );
    Draw( vertices, vertexCount, color, 255, GL_LINE_LOOP, 1 );
}

void Box2DDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    b2Vec2 vertices[2] = { p1, p2 };

    Draw( vertices, 2, color, 255, GL_LINES, 1 );
}

void Box2DDebugDraw::DrawTransform(const b2Transform& xf)
{
    b2Vec2 vertices[2] = { xf.p };

    vertices[1] = xf.p + 0.5f * xf.q.GetXAxis();
    Draw( vertices, 2, b2Color( 1, 0, 0 ), 255, GL_LINES, 1 );

    vertices[1] = xf.p + 0.5f * xf.q.GetYAxis();
    Draw( vertices, 2, b2Color( 0, 1, 0 ), 255, GL_LINES, 1 );
}

void Box2DDebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
    Draw( &p, 1, color, 255, GL_POINTS, size );
}
