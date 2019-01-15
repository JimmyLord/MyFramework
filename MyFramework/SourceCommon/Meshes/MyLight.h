//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyLight_H__
#define __MyLight_H__

#include "../DataTypes/ColorStructs.h"

enum LightTypes
{
    LightType_Directional,
    LightType_Point,
    LightType_Spot,
    LightType_NumTypes,
};

extern const char* g_LightTypeStrings[];

class MyLight : public TCPPListNode<MyLight*>
{
public:
    LightTypes m_LightType;
    Vector3 m_Position;
    Vector3 m_SpotDirectionVector; // not angles, should be normalized
    ColorFloat m_Color; // alpha ignored
    Vector3 m_Attenuation;

public:
    MyLight();
    virtual ~MyLight();
};

#endif //__MyLight_H__
