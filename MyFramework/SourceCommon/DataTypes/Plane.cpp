//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

void Plane::Set(Vector3 n, Vector3 p)
{
    m_Normal = n;
    m_Normal.Normalize();
    
    m_Distance = -m_Normal.Dot( p );
}

bool Plane::IntersectRay(Vector3 raystart, Vector3 raydir, Vector3* result)
{
    // t = -(Po dot N + d) / (V dot N)
    // P = Po + tV

    // if ray is parallel to plane, no single intersection.
    float normdotray = m_Normal.Dot( raydir );
    if( fequal( normdotray, 0 ) )
        return false;

    // solve for t
    float t = -( ( raystart.Dot(m_Normal) + m_Distance ) / normdotray );

    // solve for P
    Vector3 p = raystart + raydir * t;

    *result = p;
    return true;
}
