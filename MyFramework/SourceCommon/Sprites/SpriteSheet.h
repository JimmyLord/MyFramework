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

#ifndef __SpriteSheet_H__
#define __SpriteSheet_H__

class MySprite;
class MyFileObject;
class TextureDefinition;

class SpriteSheet
{
public:
    bool m_FullyLoaded;

    char* m_pSpriteNames;
    Vector4* m_pSpriteUVs;
    MySpritePtr* m_pSprites;
    int m_NumSprites;
    MyFileObject* m_pJSONFile;
    TextureDefinition* m_pTextureDef;

    float m_SpriteScale;
    float m_SubspriteStartX;
    float m_SubspriteEndX;
    float m_SubspriteStartY;
    float m_SubspriteEndY;

public:
    SpriteSheet();
    virtual ~SpriteSheet();
    
    void SetScale(float scale) { m_SpriteScale = scale; }
    virtual void Load(const char* basefilename, int minfilter, int magfilter, int wraps = GL_CLAMP_TO_EDGE, int wrapt = GL_CLAMP_TO_EDGE);
    virtual void Tick(double TimePassed);

    virtual void CreateSprites(int numsprites);

    int GetSpriteIndexByName(const char* name, ...);
    MySprite* GetSpriteByName(const char* name, ...);

    void CreateNewSpritesFromOtherSheet(SpriteSheet* sourcesheet, float sx, float ex, float sy, float ey);
};

#endif //__SpriteSheet_H__
