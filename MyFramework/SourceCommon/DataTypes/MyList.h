//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyList_H__
#define __MyList_H__

// Basic unordered list, doesn't grow.
template <class MyType> class MyList
{
protected:
    MyType* m_Objects;

    unsigned int m_ElementsAllocated;
    unsigned int m_ElementsInUse;

public:
    MyList()
    {
        m_Objects = 0;
        m_ElementsAllocated = 0;
        m_ElementsInUse = 0;
    }
    MyList(unsigned int length)
    {
        m_Objects = 0;
        AllocateObjects( length );
    }

    ~MyList()
    {
        FreeAllInList();
    }

    void AllocateObjects(int length)
    {
        MyAssert( m_Objects == 0 );
        if( length > 0 )
            m_Objects = MyNew MyType[length];
        m_ElementsAllocated = length;
        m_ElementsInUse = 0;
    }

    void FreeAllInList()
    {
        SAFE_DELETE_ARRAY( m_Objects );
        m_ElementsAllocated = 0;
        m_ElementsInUse = 0;
    }

    bool InsertAtIndex(unsigned int index, MyType pObj)
    {
        MyAssert( m_ElementsInUse < m_ElementsAllocated );
        if( m_ElementsInUse >= m_ElementsAllocated )
            return false;

        MyAssert( index <= m_ElementsInUse );
        if( index > m_ElementsInUse )
            return false;

        // Shift all entries after 'index' up by one.
        for( unsigned int i=m_ElementsInUse; i>index; i-- )
        {
            m_Objects[i] = m_Objects[i-1];
        }

        // Insert new object.
        m_Objects[index] = pObj;
        m_ElementsInUse++;

        return true;
    }

    bool Add(MyType pObj)
    {
        MyAssert( m_ElementsInUse < m_ElementsAllocated );
        if( m_ElementsInUse >= m_ElementsAllocated )
            return false;

        m_Objects[m_ElementsInUse] = pObj;
        m_ElementsInUse++;

        return true;
    }

    bool Remove(MyType pObj)
    {
        unsigned int i;

        for( i=0; i<m_ElementsInUse; i++ )
        {
            if( m_Objects[i] == pObj )
            {
                RemoveIndex( i );
                return true;
            }
        }

        return false;
    }

    bool Remove_MaintainOrder(MyType pObj)
    {
        unsigned int i;

        for( i=0; i<m_ElementsInUse; i++ )
        {
            if( m_Objects[i] == pObj )
            {
                RemoveIndex_MaintainOrder( i );
                return true;
            }
        }

        return false;
    }

    MyType RemoveIndex(unsigned int i)
    {
        MyType temp = m_Objects[i];
        m_Objects[i] = m_Objects[m_ElementsInUse-1];
        m_ElementsInUse--;
        return temp;
    }

    MyType RemoveIndex_MaintainOrder(unsigned int i)
    {
        MyType temp = m_Objects[i];
        for( ; i<m_ElementsInUse-1; i++ )
        {
            m_Objects[i] = m_Objects[i+1];
        }

        m_ElementsInUse--;

        return temp;
    }

    unsigned int Length()
    {
        return m_ElementsAllocated;
    }

    unsigned int Count() const
    {
        return m_ElementsInUse;
    }

    // This exists to keep code using std::vector in editor mode slightly less ifdefy.
    unsigned int size() const
    {
        return m_ElementsInUse;
    }

    MyType& operator[](unsigned int i) const
    {
        MyAssert( i < m_ElementsInUse );
        return m_Objects[i];
    }

    void BlockFill(const void* ptr, unsigned int size, unsigned int count)
    {
        MyAssert( m_ElementsInUse == 0 );
        MyAssert( size == sizeof(MyType) * count );
        memcpy( m_Objects, ptr, size );
        m_ElementsInUse = count;
    }

    void Clear()
    {
        m_ElementsInUse = 0;
    }
};

#endif //__MyList_H__
