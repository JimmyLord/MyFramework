//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyList_H__
#define __MyList_H__

// TODO: this class is awful, needs to be a linked list.

template <class MyType> class MyList
{
protected:
    MyType* m_Objects;

    unsigned int m_Length; // num elements allocated in list
    unsigned int m_Count; // num elements used.

public:
    //Construction and destruction
    MyList()
    {
        m_Objects = 0;
        m_Length = 0;
        m_Count = 0;
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
        assert( m_Objects == 0 );
        if( length > 0 )
            m_Objects = MyNew MyType[length];
        m_Length = length;
        m_Count = 0;
    }

    void FreeAllInList()
    {
        SAFE_DELETE_ARRAY( m_Objects );
        m_Length = 0;
        m_Count = 0;
    }

    bool InsertAtIndex(unsigned int index, MyType pObj)
    {
        assert( m_Count < m_Length );
        if( m_Count >= m_Length )
            return false;

        assert( index <= m_Count );
        if( index > m_Count )
            return false;

        for( unsigned int i=m_Count; i>index; i-- )
        {
            m_Objects[i] = m_Objects[i-1];
        }

        m_Objects[index] = pObj;
        m_Count++;

        return true;
    }

    bool Add(MyType pObj)
    {
        assert( m_Count < m_Length );
        if( m_Count >= m_Length )
            return false;

        m_Objects[m_Count] = pObj;
        m_Count++;

        return true;
    }

    bool Remove(MyType pObj)
    {
        unsigned int i;

        for( i=0; i<m_Count; i++ )
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

        for( i=0; i<m_Count; i++ )
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
        m_Objects[i] = m_Objects[m_Count-1];
        m_Count--;
        return temp;
    }

    MyType RemoveIndex_MaintainOrder(unsigned int i)
    {
        MyType temp = m_Objects[i];
        for( ; i<m_Count-1; i++ )
        {
            m_Objects[i] = m_Objects[i+1];
        }

        m_Count--;

        return temp;
    }

    unsigned int Length()
    {
        return m_Length;
    }

    unsigned int Count() const
    {
        return m_Count;
    }

    MyType& operator[] (int i) const
    {
        return m_Objects[i];
    }

    void BlockFill(void* ptr, unsigned int size, unsigned int count)
    {
        assert( m_Count == 0 );
        assert( size == sizeof(MyType) * count );
        memcpy( m_Objects, ptr, size );
        m_Count = count;
    }

    void Clear()
    {
        m_Count = 0;
    }
};

#endif //__MyList_H__
