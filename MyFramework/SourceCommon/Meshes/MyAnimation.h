//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyAnimation_H__
#define __MyAnimation_H__

class MyChannel
{
public:
    int m_NodeIndex;

    MyList<float> m_TranslationTimes;
    MyList<Vector3> m_TranslationValues;

    MyList<float> m_RotationTimes;
    MyList<MyQuat> m_RotationValues;

    MyList<float> m_ScaleTimes;
    MyList<Vector3> m_ScaleValues;

public:
    int ImportFromBuffer(char* pBuffer, float scale);
};

class MyAnimation
{
    static const int MAX_ANIMATION_NAME_LEN = 32;
public:
    char m_Name[MAX_ANIMATION_NAME_LEN+1];
    int m_TimelineIndex;

    float m_StartTime;
    float m_Duration;

public:
    void SetName(const char* name)
    {
        if( strlen(name) > (unsigned int)MAX_ANIMATION_NAME_LEN )
            LOGInfo( LOGTag, "Warning: animation name longer than 32 characters - %s - truncating\n", name );
        strncpy_s( m_Name, MAX_ANIMATION_NAME_LEN+1, name, MAX_ANIMATION_NAME_LEN );
    }
};

class MyAnimationTimeline
{
public:
    char* m_Name;
    float m_Duration;
    float m_TicksPerSecond;

    MyChannel* m_pChannelMemoryAllocation;
    MyList<MyChannel*> m_pChannels;

public:
    MyAnimationTimeline();
    virtual ~MyAnimationTimeline();

    void ImportFromJSON(cJSON* pAnimObj);
    int ImportChannelsFromBuffer(char* pBuffer, float scale);

    void SetNumberOfChannels(unsigned int numchannels);

    int FindChannelIndexForNode(unsigned int nodeindex);

    Vector3 GetInterpolatedTranslation(float time, unsigned int channelindex);
    MyQuat GetInterpolatedRotation(float time, unsigned int channelindex);
    Vector3 GetInterpolatedScaling(float time, unsigned int channelindex);
};

#endif //__MyAnimation_H__
