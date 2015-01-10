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

    inline void Set(Vector3 n, float d) { m_Normal = n; m_Normal.Normalize(); m_Distance = d; }
    void Set(Vector3 n, Vector3 p);

    bool IntersectRay(Vector3 raystart, Vector3 raydir, Vector3* result);
};

#endif //__Plane_H__
