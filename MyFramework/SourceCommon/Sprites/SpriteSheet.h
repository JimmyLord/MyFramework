//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SpriteSheet_H__
#define __SpriteSheet_H__

class MySprite;
class MyFileObject;
class TextureDefinition;

class SpriteSheet
{
protected:
    bool m_FullyLoaded;

    bool m_CreateSprites; // optionally create a MySprite for each image in the sheet (with UVs baked into the verts)
    bool m_CreateMaterials; // or create a MaterialDefinition for each image holding it's uv scale/offset
                            // or neither or both

    char* m_pSpriteNames;
    Vector4* m_pSpriteUVs;
    int m_NumSprites;
    MyFileObject* m_pJSONFile;
    MaterialDefinition* m_pMaterial;

    MySpritePtr* m_pSprites;
    MaterialDefinition** m_pMaterialList;

    float m_SpriteScale;
    float m_SubspriteStartX;
    float m_SubspriteEndX;
    float m_SubspriteStartY;
    float m_SubspriteEndY;

protected:
    virtual void CreateSprites();
    virtual void CreateMaterials();

public:
    SpriteSheet();
    virtual ~SpriteSheet();
    
    void SetScale(float scale) { m_SpriteScale = scale; }
    virtual void Create(const char* fullpath, ShaderGroup* pShader, int minfilter, int magfilter, bool createsprites, bool creatematerials);
    //virtual void Create(MaterialDefinition* pMaterial, bool createsprites, bool creatematerials);
    virtual void Tick(double TimePassed);

    int GetSpriteIndexByName(const char* name, ...);
    MySprite* GetSpriteByName(const char* name, ...);

    void CreateNewSpritesFromOtherSheet(SpriteSheet* sourcesheet, float sx, float ex, float sy, float ey);

    Vector4 GetSpriteUVs(int index) const { return m_pSpriteUVs[index]; }

    MaterialDefinition* GetSheetMaterial() { return m_pMaterial; }
    MaterialDefinition* GetSpriteMaterial(int index) { MyAssert( m_pMaterialList ); return m_pMaterialList[index]; }

    bool IsFullyLoaded() { return m_FullyLoaded; }
};

#endif //__SpriteSheet_H__
