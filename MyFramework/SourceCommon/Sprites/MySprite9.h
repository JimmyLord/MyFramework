//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MySprite9_H__
#define __MySprite9_H__

class BaseShader;
class TextureDefinition;
struct Vertex_Base;

class MySprite9
{
protected:
    ShaderGroup* m_pShaderGroup;
    TextureDefinition* m_pTexture;

    //Vector2 m_SpriteSize;

    BufferDefinition* m_pVertexBuffer;
    BufferDefinition* m_pIndexBuffer;
    VAODefinition* m_pVAO;

    MyMatrix m_Position;
    MyMatrix* m_pParentMatrix;

    ColorByte m_Tint;

public:
    MySprite9();
    virtual ~MySprite9();

    void Create(float x1, float x2, float x3, float x4, float y1, float y2, float y3, float y4, float u1, float u2, float u3, float u4, float v1, float v2, float v3, float v4);

    virtual void SetShader(ShaderGroup* pShaderGroup);
    virtual void SetShaderAndTexture(ShaderGroup* pShaderGroup, TextureDefinition* pTexture);

    //virtual void Create(float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags);
    //virtual void CreateSubsection(float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, float spx = 0, float epx = 1, float spy = 0, float epy = 1);
    MyMatrix GetPosition();
    void SetPosition(Vector3 pos, bool setindentity = true);
    void SetPosition(float x, float y, float z, bool setindentity = true);
    void SetPosition(MyMatrix* mat);
    //void SetZRotation(float rotation, bool preserveposition = true, Vector3* localpivot = 0);
    //void SetRST(Vector3 rot, Vector3 scale, Vector3 pos, bool setindentity = true);
    //void SetTransform(MyMatrix& mat);
    //ColorByte* GetTintPointer() { return &m_Tint; }
    void SetTint(ColorByte tintcolor);
    //virtual void FlipX();

    //virtual bool Setup(MyMatrix* matviewproj);
    //virtual void DrawNoSetup();
    //virtual void DeactivateShader();
    virtual void Draw(MyMatrix* matviewproj);

    //Vertex_Base* GetVerts(bool markdirty);

    //int GetTextureID();
    //ShaderGroup* GetShader() { return m_pShaderGroup; }
    //ColorByte GetTint() { return m_Tint; }
};

#endif //__MySprite9_H__
