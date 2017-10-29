//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyAABounds_H__
#define __MyAABounds_H__

class MyAABounds
{
protected:
    Vector3 m_Center;
    Vector3 m_HalfSize;
    float m_Radius; // Largest half-size

public:
    MyAABounds() {}

    Vector3 GetCenter() { return m_Center; }
    Vector3 GetHalfSize() { return m_HalfSize; }
    float GetRadius() { return m_Radius; }

    void Set(Vector3 c, Vector3 h)
    {
        m_Center = c;
        m_HalfSize = h;
        if( m_HalfSize.x > m_HalfSize.y )
            m_Radius = m_HalfSize.x > m_HalfSize.z ? m_HalfSize.x : m_HalfSize.z;
        else
            m_Radius = m_HalfSize.y > m_HalfSize.z ? m_HalfSize.y : m_HalfSize.z;
    }

    bool operator ==(const MyAABounds o)
    {
        if( m_Center != o.m_Center )
            return false;
        if( m_HalfSize != o.m_HalfSize )
            return false;
        if( fequal( m_Radius, o.m_Radius ) )
            return false;

        return true;
    }

    bool operator !=(const MyAABounds o)
    {
        if( m_Center != o.m_Center )
            return true;
        if( m_HalfSize != o.m_HalfSize )
            return true;
        if( fequal( m_Radius, o.m_Radius ) == false )
            return true;

        return false;
    }
};

#endif //__MyAABounds_H__
