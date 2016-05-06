//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __AnimatedSprite_H__
#define __AnimatedSprite_H__

class KeyTranslation;
class KeyScale;
class KeyAlpha;
class KeyRotation;

class AnimatedSpriteAnchorPoint
{
public:
    Vector3 point;
    bool flipx;

public:
    void Clear()
    {
        point.Set( 0, 0, 0 );
        flipx = false;
    }
};

class AnimatedSpriteDefinition
{
public:
    static const unsigned int m_MaxFrames = 20;
    static const unsigned int m_MaxKeys = 10;

    MySprite* m_pSprites[m_MaxFrames];
    double m_TimeToHoldFrame[m_MaxFrames];
    unsigned int m_NumFrames;

    double m_TimeOfLastKey;

    bool m_Looping;

    MyList<KeyTranslation*> m_KeysTranslation;
    MyList<KeyScale*> m_KeysScale;
    MyList<KeyAlpha*> m_KeysAlpha;
    MyList<KeyRotation*> m_KeysRotation;

    AnimatedSpriteAnchorPoint* m_pAnchorPoints;
    unsigned int m_NumAnchorFrames; // for asserting.
    unsigned int m_NumAnchorsPerFrame;

public:
    AnimatedSpriteDefinition(int numframes, int numanchorsperframe);
    ~AnimatedSpriteDefinition();

    void SetLooping(bool value) { m_Looping = value; }

    void SetSpriteForFrame(unsigned int framenum, MySprite* pSprite, double holdtime);
    void SetAnchorsForFrame(unsigned int frame, AnimatedSpriteAnchorPoint* pVectors, unsigned int numanchors);
    void SetAnchorsForFrame(unsigned int frame, Vector3* pAnchors, unsigned int numanchors);    

    void AddKeyTranslation(double time, Vector3 position);
    void AddKeyScale(double time, Vector3 scale);
    void AddKeyAlpha(double time, float alpha);
    void AddKeyRotation(double time, Vector3 rotation);

    unsigned int GetFrame(double time);
    MySprite* GetSprite(double time);
    AnimatedSpriteAnchorPoint* GetAnchor(double time, unsigned int anchornum);
    Vector3 GetPosition(double time);
    Vector3 GetScale(double time);
    float GetAlpha(double time);
    Vector3 GetRotation(double time);
};

class AnimatedSpriteInstance
{
public:
    AnimatedSpriteDefinition* m_pAnimDef;

    double m_TimeFromStart;

    MyMatrix m_BaseTransform;
    MyMatrix* m_pParentMatrix;

    MyMatrix m_WorldTransform;

public:
    AnimatedSpriteInstance();
    ~AnimatedSpriteInstance();

    void SetSpriteAnimDef( AnimatedSpriteDefinition* anim ) { m_pAnimDef = anim; }

    void Clear();
    void ResetAnimation();
    void Tick(double TimePassed);

    MySprite* SetupSpriteForDrawing(MySprite* overridesprite);
    void Draw(MyMatrix* matviewproj);

    void SetTime(double time);

    void SetBaseTransform(MyMatrix& mat);

    bool IsFinished();
    AnimatedSpriteAnchorPoint* GetAnchor(unsigned int anchornum);
};

#endif //__AnimatedSprite_H__
