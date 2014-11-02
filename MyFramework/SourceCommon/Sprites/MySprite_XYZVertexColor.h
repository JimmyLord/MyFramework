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

#ifndef __MySprite_XYZVertexColor_H__
#define __MySprite_XYZVertexColor_H__

class Shader_TextureVertexColor;
class TextureDefinition;
struct Vertex_Sprite;

class MySprite_XYZVertexColor;
typedef MySprite_XYZVertexColor* MySprite_XYZVertexColorPtr;

class MySprite_XYZVertexColor : public MySprite
{
protected:

public:
    MySprite_XYZVertexColor();
    virtual ~MySprite_XYZVertexColor();

    virtual void Create(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts = false);
    virtual void Create(float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, bool staticverts = false);
    virtual void CreateSubsection(const char* category, float spritew, float spriteh, float startu, float endu, float startv, float endv, unsigned char justificationflags, float spx = 0, float epx = 1, float spy = 0, float epy = 1, bool staticverts = false);

    virtual void FlipX();

    //virtual bool Setup(MyMatrix* matviewproj);
    //virtual void DrawNoSetup();
    //virtual void DeactivateShader();
    virtual void Draw(MyMatrix* matviewproj);

    virtual void SetVertexColors( ColorByte bl, ColorByte br, ColorByte tl, ColorByte tr );
    virtual void SetVertexColors( ColorFloat bl, ColorFloat br, ColorFloat tl, ColorFloat tr );
};

#endif //__MySprite_XYZVertexColor_H__
