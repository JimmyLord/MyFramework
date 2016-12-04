//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __My2DAnimInfo_H__
#define __My2DAnimInfo_H__

class My2DAnimationFrame
{
    friend class My2DAnimation;
    friend class My2DAnimInfo;

protected:
    MaterialDefinition* m_pMaterial;
    float m_Duration;
    Vector2 m_UVScale;
    Vector2 m_UVOffset;

public:
    My2DAnimationFrame();
    ~My2DAnimationFrame();

    MaterialDefinition* GetMaterial() { return m_pMaterial; }
    float GetDuration() { return m_Duration; }
    Vector2 GetUVScale() { return m_UVScale; }
    Vector2 GetUVOffset() { return m_UVOffset; }

    void SetMaterial(MaterialDefinition* pMaterial);
};

class My2DAnimation
{
    friend class My2DAnimInfo;

protected:
    static const int MAX_ANIMATION_NAME_LEN = 32;

    char m_Name[MAX_ANIMATION_NAME_LEN+1];
    MyList<My2DAnimationFrame*> m_Frames;

public:
    void SetName(const char* name);

    uint32 GetFrameCount();
    My2DAnimationFrame* GetFrameByIndex(uint32 frameindex);
    My2DAnimationFrame* GetFrameByIndexClamped(uint32 frameindex);
};

class My2DAnimInfo
{
    static const unsigned int MAX_ANIMATIONS = 10; // TODO: fix this hardcodedness
    static const unsigned int MAX_FRAMES_IN_ANIMATION = 10; // TODO: fix this hardcodedness

protected:
    MyFileObject* m_pSourceFile;
    MyList<My2DAnimation*> m_Animations;

public:
    My2DAnimInfo();
    virtual ~My2DAnimInfo();

    uint32 GetNumberOfAnimations();
    My2DAnimation* GetAnimationByIndex(uint32 animindex);
    My2DAnimation* GetAnimationByIndexClamped(uint32 animindex);

#if MYFW_USING_WX
    void SaveAnimationControlFile();
    void LoadAnimationControlFile(char* buffer);
#endif
};

#endif //__My2DAnimInfo_H__
