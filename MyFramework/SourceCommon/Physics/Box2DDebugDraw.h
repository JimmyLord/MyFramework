//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Box2DDebugDraw_H__
#define __Box2DDebugDraw_H__

class Box2DDebugDraw : public b2Draw
{
public:
    MyMesh* m_pMesh;
    MyMatrix* m_pMatViewProj;
    VertexFormat_Dynamic_Desc* m_pVertexFormatDesc;

public:
    Box2DDebugDraw(MaterialDefinition* debugdrawmaterial, MyMatrix* matviewproj);
    ~Box2DDebugDraw();

    virtual void Draw(const b2Vec2* vertices, int32 vertexCount, const b2Color& color, unsigned char alpha, int primitivetype);
    virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color); // CCW order.
    virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color); // CCW order.
    virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
    virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
    virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
    virtual void DrawTransform(const b2Transform& xf);
};

#endif //__Box2DDebugDraw_H__
