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

#ifndef __MySprite_H__
#define __MySprite_H__

class BaseShader;
class TextureDefinition;
class BufferDefinition;
struct Vertex_Base;
class ShaderGroup;

class MySprite;
typedef MySprite* MySpritePtr;

class MySprite : public RefCount
{
protected:
    bool m_SpriteIsStatic;

public:
    ShaderGroup* m_pShaderGroup;
    TextureDefinition* m_pTexture;
    int m_TextureID;

    // store up the current size and texcoords, if these change we need to rebuild sprite
    Vector2 m_SpriteSize;
    Vector2 m_SpriteUVStart;
    Vector2 m_SpriteUVEnd;
    unsigned char m_SpriteJustification;

    BufferDefinition* m_pVertexBuffer;
    BufferDefinition* m_pIndexBuffer;
    VAODefinition* m_pVAO;

    MyMatrix m_Position;
    MyMatrix* m_pParentMatrix;

    ColorByte m_Tint;

public:
    MySprite();
    MySprite(MySprite* pSprite, const char* category);
    virtual ~MySprite();

    virtual void SetShader(ShaderGroup* pShaderGroup);
    virtual void SetShaderAndTexture(ShaderGroup* pShaderGroup, TextureDefinition* pTexture);
    virtual void SetShaderAndTexture(ShaderGroup* pShaderGroup, int texid);

    virtual void Create(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts = false);
    virtual void Create(float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts = false);
    virtual void CreateSubsection(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, float spx = 0, float epx = 1, float spy = 0, float epy = 1, bool staticverts = false);
    MyMatrix GetPosition();
    void SetPosition(Vector3 pos, bool setindentity = true);
    void SetPosition(float x, float y, float z, bool setindentity = true);
    void SetPosition(MyMatrix* mat);
    void SetZRotation(float rotation, bool preserveposition = true, Vector3* localpivot = 0);
    void SetRST(Vector3 rot, Vector3 scale, Vector3 pos, bool setindentity = true);
    void SetTransform(MyMatrix& mat);
    ColorByte* GetTintPointer() { return &m_Tint; }
    void SetTint(ColorByte tintcolor);
    virtual void FlipX();

    virtual bool Setup(MyMatrix* matviewproj);
    virtual void DrawNoSetup();
    virtual void DeactivateShader();
    virtual void Draw(MyMatrix* matviewproj);

    Vertex_Base* GetVerts(bool markdirty);

    int GetTextureID();
    ShaderGroup* GetShader() { return m_pShaderGroup; }
    ColorByte GetTint() { return m_Tint; }
};

#endif //__MySprite_H__
