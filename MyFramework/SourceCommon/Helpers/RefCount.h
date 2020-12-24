//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __RefCount_H__
#define __RefCount_H__

#include "../Helpers/MyAssert.h"

class RefCount
{
protected:
    unsigned int m_RefCount;
#if _DEBUG
    unsigned int m_BaseCount;
#endif

public:
    RefCount()
    {
        m_RefCount = 1;
#if _DEBUG
        m_BaseCount = 0;
#endif
    }
    virtual ~RefCount()
    {
        MyAssert( m_RefCount == m_BaseCount );
    }

#if _DEBUG
    void Debug_SetBaseCount(unsigned int count) { m_BaseCount = count; }
#endif

    unsigned int GetRefCount()
    {
        return m_RefCount;
    }

    void AddRef()
    {
        m_RefCount++;
    }
    virtual void Release()
    {
        m_RefCount--;
        if( m_RefCount == 0 )
            delete this;
    }

#if _DEBUG
    void RemoveFinalRefIfCreatedOnStackToAvoidAssertInDestructor()
    {
        MyAssert( m_RefCount == 1 );
        m_RefCount--;
    }
#else
    void RemoveFinalRefIfCreatedOnStackToAvoidAssertInDestructor()
    {
        // TODO: is there a reasonable way to remove this function call altogether...
    }
#endif
};

#endif //__RefCount_H__
