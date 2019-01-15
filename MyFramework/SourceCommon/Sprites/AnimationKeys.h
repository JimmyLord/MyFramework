//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __AnimationKeys_H__
#define __AnimationKeys_H__

#include "../DataTypes/MyActivePool.h"

class AnimationKeyPool;
template <class MyType> class MyActivePool;
class KeyFrame;
class KeyTranslation;
class KeyScale;
class KeyAlpha;
class KeyRotation;

extern AnimationKeyPool* g_pAnimationKeyPool;

class AnimationKeyPool
{
public:
    MyActivePool<KeyTranslation*> m_TranslationKeys;
    MyActivePool<KeyScale*> m_ScaleKeys;
    MyActivePool<KeyAlpha*> m_AlphaKeys;
    MyActivePool<KeyRotation*> m_RotationKeys;

public:
    AnimationKeyPool();
    ~AnimationKeyPool();
};

enum KeyTypes
{
    KeyType_None,
    KeyType_Translation,
    KeyType_Scale,
    KeyType_Alpha,
    KeyType_Rotation,
};

class KeyFrame
{
public:
    double m_Time;
    KeyTypes m_Type;

    KeyFrame()
    {
        m_Time = 0;
        m_Type = KeyType_None;
    }
};

class KeyTranslation : public KeyFrame
{
public:
    Vector3 m_Position;

    KeyTranslation()
    : m_Position(0,0,0)
    {
        m_Type = KeyType_Translation;
    }
};

class KeyScale : public KeyFrame
{
public:
    Vector3 m_Scale;

    KeyScale()
    : m_Scale(0,0,0)
    {
        m_Type = KeyType_Scale;
    }
};

class KeyAlpha : public KeyFrame
{
public:
    float m_Alpha;

    KeyAlpha()
    : m_Alpha(0)
    {
        m_Type = KeyType_Alpha;
    }
};

class KeyRotation : public KeyFrame
{
public:
    Vector3 m_Rotation;

    KeyRotation()
    : m_Rotation(0,0,0)
    {
        m_Type = KeyType_Rotation;
    }
};

#endif //__AnimationKeys_H__
