//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SpriteSheet_XYZVertexColor_H__
#define __SpriteSheet_XYZVertexColor_H__

class MySprite;
class MyFileObject;
class TextureDefinition;

class SpriteSheet_XYZVertexColor : public SpriteSheet
{
public:

public:
    SpriteSheet_XYZVertexColor()
    {
    }

    virtual ~SpriteSheet_XYZVertexColor()
    {
    }
    
    virtual void CreateSprites()
    {
        m_pSprites = (MySpritePtr*)MyNew MySprite_XYZVertexColorPtr[m_NumSprites];
        for( int i=0; i<m_NumSprites; i++ )
        {
            m_pSprites[i] = MyNew MySprite_XYZVertexColor( false );
            m_pSprites[i]->SetMaterial( m_pMaterial );
        }
    }
};

#endif //__SpriteSheet_XYZVertexColor_H__
