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
    int m_Index;

    MyList<float> m_TranslationTimes;
    MyList<Vector3> m_TranslationValues;

    MyList<float> m_RotationTimes;
    MyList<Vector4> m_RotationValues; // TODO: make a list of quaternions.

    MyList<float> m_ScaleTimes;
    MyList<Vector3> m_ScaleValues;

public:
    int ImportFromBuffer(char* pBuffer);
};

class MyAnimation
{
public:
    char* m_Name;
    float m_Duration;
    float m_TicksPerSecond;

    MyChannel* m_pChannelMemoryAllocation;
    MyList<MyChannel*> m_pChannels;

public:

public:
    MyAnimation();
    virtual ~MyAnimation();

    void ImportFromJSON(cJSON* pAnimObj);
    int ImportChannelsFromBuffer(char* pBuffer);

    void SetNumberOfChannels(unsigned int numchannels);

    Vector3 GetInterpolatedTranslation(float time, unsigned int nodeindex);
    Vector4 GetInterpolatedRotation(float time, unsigned int nodeindex);
    Vector3 GetInterpolatedScaling(float time, unsigned int nodeindex);
};

#endif //__MyAnimation_H__
