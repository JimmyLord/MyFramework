//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyActivePool_H__
#define __MyActivePool_H__

template <class MyType> class MySimplePool
{
protected:
    MyType* m_pObjects;
    MyType** m_pObjectPtrs;

    unsigned int m_Length; // num elements allocated in list
    unsigned int m_Count; // num elements used.

public:
    MySimplePool()
    {
        m_pObjects = 0;
        m_pObjectPtrs = 0;

        m_Length = 0;
        m_Count = 0;
    }

    ~MySimplePool()
    {
        MyAssert( m_Count == m_Length );
        SAFE_DELETE_ARRAY( m_pObjects );
        SAFE_DELETE_ARRAY( m_pObjectPtrs );
    }

    unsigned int GetLength() { return m_Length; }
    unsigned int GetCount() { return m_Count; }
    unsigned int GetNumUsed() { return m_Length - m_Count; }

    bool IsInitialized()
    {
        return m_Length != 0 ? true : false;
    }

    void AllocateObjects(unsigned int length)
    {
        MyAssert( m_pObjects == 0 );
        
        if( length > 0 )
        {
            m_pObjects = MyNew MyType[length];
            m_pObjectPtrs = MyNew MyType*[length];
        }

        for( unsigned int i=0; i<length; i++ )
        {
            m_pObjectPtrs[i] = &m_pObjects[i];
        }

        m_Length = length;
        m_Count = length;
    }

    MyType* GetObject()
    {
        if( m_Count == 0 )
        {
            LOGInfo( LOGTag, "WARNING: MySimplePool is empty\n" );
            return 0;
        }

        m_Count--;
        return m_pObjectPtrs[m_Count];
    }

    void ReturnObject(MyType* object)
    {
        MyAssert( m_Count < m_Length );

        m_pObjectPtrs[m_Count] = object;
        m_Count++;
    }

#if _DEBUG
    unsigned int Debug_GetLength() { return m_Length; }
    MyType& operator[] (unsigned int i) const
    {
        MyAssert( i < m_Count );
        return *m_pObjectPtrs[i];
    }
#endif
};

template <class MyType> class MyActivePool
{
public:
    MyList<MyType> m_ActiveObjects;
    MyList<MyType> m_InactiveObjects;

public:
    //Construction and destruction
    MyActivePool(unsigned int length)
    : m_ActiveObjects(length)
    , m_InactiveObjects(length)
    {
    }
    MyActivePool()
    {
    }
    ~MyActivePool()
    {
    }

    void AllocateObjects(unsigned int length)
    {
        MyAssert( m_ActiveObjects.Length() == 0 );
        m_ActiveObjects.AllocateObjects( length );
        m_InactiveObjects.AllocateObjects( length );
    }

    void DeallocateObjectLists()
    {
        m_ActiveObjects.FreeAllInList();
        m_InactiveObjects.FreeAllInList();
    }

    unsigned int Length()
    {
        return m_ActiveObjects.Length();
    }

    void AddInactiveObject(MyType pObj)
    {
        m_InactiveObjects.Add( pObj );
    }

    MyType MakeObjectActive(int destindex = -1)
    {
        if( m_InactiveObjects.Count() <= 0 )
            return 0;

        MyType pObj = m_InactiveObjects.RemoveIndex( m_InactiveObjects.Count()-1 );

        if( destindex == -1 )
            m_ActiveObjects.Add( pObj );
        else
            m_ActiveObjects.InsertAtIndex( destindex, pObj );

        return pObj;
    }

    void MakeObjectInactive(MyType pObj, bool maintainorder = false)
    {
        bool removed = false;

        if( maintainorder )
            removed = m_ActiveObjects.Remove_MaintainOrder( pObj );
        else
            removed = m_ActiveObjects.Remove( pObj );

        MyAssert( removed == true );

        m_InactiveObjects.Add( pObj );
    }

    void MakeObjectInactiveByIndex(unsigned int index, bool maintainorder = false)
    {
        MyType pObj = 0;
        if( maintainorder )
            pObj = m_ActiveObjects.RemoveIndex_MaintainOrder( index );
        else
            pObj = m_ActiveObjects.RemoveIndex( index );
        m_InactiveObjects.Add( pObj );
    }

    void InactivateAllObjects()
    {
        for( unsigned int i=0; i<m_ActiveObjects.Count(); i++ )
        {
            MyType pObj = m_ActiveObjects.RemoveIndex( i );
            m_InactiveObjects.Add( pObj );
            i--;
        }
    }

    void DeleteAllObjectsInPool()
    {
        for( unsigned int i=0; i<m_ActiveObjects.Count(); i++ )
            delete m_ActiveObjects[i];
        m_ActiveObjects.Clear();

        for( unsigned int i=0; i<m_InactiveObjects.Count(); i++ )
            delete m_InactiveObjects[i];
        m_InactiveObjects.Clear();
    }

    void DeleteAllObjectsInPoolAsArrayAllocations()
    {
        for( unsigned int i=0; i<m_ActiveObjects.Count(); i++ )
            delete[] m_ActiveObjects[i];
        m_ActiveObjects.Clear();

        for( unsigned int i=0; i<m_InactiveObjects.Count(); i++ )
            delete[] m_InactiveObjects[i];
        m_InactiveObjects.Clear();
    }
};

#endif //__MyActivePool_H__
