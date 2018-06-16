//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SpriteBatch_XYZVertexColor_H__
#define __SpriteBatch_XYZVertexColor_H__

class BaseShader;
class TextureDefinition;
struct Vertex_Base;

class SpriteBatch_XYZVertexColor : public SpriteBatch
{
protected:

public:
    SpriteBatch_XYZVertexColor();
    virtual ~SpriteBatch_XYZVertexColor();

    virtual void AllocateVertices(int numsprites);
    virtual void AddSprite(MyMatrix* pMatWorld, MySprite* pSprite);
    void AddSprite(MyMatrix* pMatWorld, MySprite_XYZVertexColor* pSprite);
    virtual void Draw(MyMatrix* pMatProj, MyMatrix* pMatView);
};

#endif //__SpriteBatch_XYZVertexColor_H__
