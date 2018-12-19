//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Plane_H__
#define __Plane_H__

class Plane
{
public:
    Vector3 m_Normal;
    float m_Distance;

public:
    Plane() {}

    inline void Set(Vector3 normal, float distance) { m_Normal = normal; m_Normal.Normalize(); m_Distance = distance; }
    void Set(Vector3 normal, Vector3 point);

    bool IntersectRay(Vector3 rayStart, Vector3 rayDir, Vector3* pResult);
};

#endif //__Plane_H__
