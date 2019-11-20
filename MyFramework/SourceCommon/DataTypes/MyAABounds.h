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

class AABB2D
{
protected:
    Vector2 m_Min;
    Vector2 m_Max;

public:
    AABB2D() {}

    void Set(const Vector2& topLeft, const Vector2& bottomRight)
    {
        m_Min = topLeft;
        m_Max = bottomRight;

        MyAssert( m_Min.x <= m_Max.x );
        MyAssert( m_Min.y <= m_Max.y );
    }

    void SetUnsorted(const Vector2& pos1, const Vector2& pos2)
    {
        if( pos1.x < pos2.x ) { m_Min.x = pos1.x; m_Max.x = pos2.x; }
                         else { m_Min.x = pos2.x; m_Max.x = pos1.x; }
        if( pos1.y < pos2.y ) { m_Min.y = pos1.y; m_Max.y = pos2.y; }
                         else { m_Min.y = pos2.y; m_Max.y = pos1.y; }
    }

    bool IsOverlapped(const AABB2D& other) const
    {
        if( this->m_Min.x > other.m_Max.x || other.m_Min.x > this->m_Max.x ) return false;
        if( this->m_Min.y > other.m_Max.y || other.m_Min.y > this->m_Max.y ) return false;

        return true;
    }

    // From: https://www.gamedev.net/forums/topic/338987-aabb---line-segment-intersection-test/
    //       and naively adapted to 2D.
    bool IntersectsRaySegment(const Vector2& rayStart, const Vector2& rayEnd) const
    {
        Vector2 d = (rayEnd - rayStart) * 0.5f;
        Vector2 e = (m_Max - m_Min) * 0.5f;
        Vector2 c = rayStart + d - (m_Min + m_Max) * 0.5f;
        Vector2 ad = d.GetAbsolute();

        if( fabsf(c[0]) > e[0] + ad[0] )
            return false;
        if( fabsf(c[1]) > e[1] + ad[1] )
            return false;
        //if( fabsf(c[2]) > e[2] + ad[2] )
        //    return false;

        //if( fabsf(d[1] * c[2] - d[2] * c[1]) > e[1] * ad[2] + e[2] * ad[1] + FEQUALEPSILON )
        //    return false;
        //if( fabsf(d[2] * c[0] - d[0] * c[2]) > e[2] * ad[0] + e[0] * ad[2] + FEQUALEPSILON )
        //    return false;
        if( fabsf(d[0] * c[1] - d[1] * c[0]) > e[0] * ad[1] + e[1] * ad[0] + FEQUALEPSILON )
            return false;

        return true;
    }
};

#endif //__MyAABounds_H__
