//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#if MYFW_WINDOWS

#define MEMORY_ShowDebugInfo   0

class AllocationList;

CPPListHead g_StaticallyAllocatedRam;
AllocationList* g_pAllocationList = 0;
unsigned int g_TotalAllocatedRam = 0;
unsigned int g_AllocatedRamCount = 0;
unsigned int g_ActiveAllocatedRamCount = 0;

#if USE_PTHREAD
pthread_mutex_t g_AllocationMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

class DummyClassToForceAStaticOperatorNew
{
public:
    int* m_DummyInt;
public:
    DummyClassToForceAStaticOperatorNew()
    {
        m_DummyInt = new int;
    }
    ~DummyClassToForceAStaticOperatorNew()
    {
        delete m_DummyInt;
    }
};
DummyClassToForceAStaticOperatorNew DummyInstanceOfAClassToForceAStaticOperatorNew;

class AllocationList
{
public:
    CPPListHead m_Allocations;

    AllocationList()
    {
        int bp = 1;
    }

    ~AllocationList()
    {
        MyMemory_ValidateAllocations( this, true );
    }
};

void MyMemory_ValidateAllocations(AllocationList* pList, bool AssertOnAnyAllocation)
{
#if MYFW_WINDOWS && _DEBUG
    // grab the mutex, for g_pAllocationList and other tracking globals
    pthread_mutex_lock( &g_AllocationMutex );

    LOGInfo( LOGTag, "Start dumping unfreed memory allocations.\n" );
    CPPListNode* pNode;
    for( pNode = pList->m_Allocations.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        MemObject* obj = (MemObject*)pNode;
        MyAssert( obj->m_size < 200000 );
        MyAssert( obj->m_type < 3 );
        MyAssert( obj->m_line < 2500 );
        MyAssert( obj->Next != NULL );
        MyAssert( obj->Prev != NULL );

        // Pointer to allocation not freed: ((char*)obj)+sizeof(MemObject)

        LOGInfo( LOGTag, "%s(%d):(%d) %d bytes : Memory unreleased.\n", obj->m_file, obj->m_line, obj->m_allocationcount, obj->m_size );
    }
    LOGInfo( LOGTag, "End dumping unfreed memory allocations.\n" );

    if( AssertOnAnyAllocation && pList->m_Allocations.GetHead() != 0 )
        MyAssert( false );

    pthread_mutex_unlock( &g_AllocationMutex );
#endif
}

size_t MyMemory_GetNumberOfBytesAllocated()
{
    if( g_pAllocationList == 0 )
        return 0;

#if MYFW_WINDOWS && _DEBUG
    // grab the mutex, for g_pAllocationList and other tracking globals
    pthread_mutex_lock( &g_AllocationMutex );

    // since the list itself isn't in the list, start with it's size.
    size_t count = sizeof( AllocationList );

    CPPListNode* pNode;
    for( pNode = g_pAllocationList->m_Allocations.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        MemObject* obj = (MemObject*)pNode;
        count += obj->m_size;
    }

    for( pNode = g_StaticallyAllocatedRam.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        MemObject* obj = (MemObject*)pNode;
        count += obj->m_size;
    }

    //LOGInfo( LOGTag, "Memory used %d\n", count );
    MyAssert( count == g_TotalAllocatedRam );

    pthread_mutex_unlock( &g_AllocationMutex );

    return count;
#else
    return 0;
#endif
}

unsigned int MyMemory_GetNumberOfMemoryAllocations()
{
    return g_AllocatedRamCount;
}

unsigned int MyMemory_GetNumberOfActiveMemoryAllocations()
{
    return g_ActiveAllocatedRamCount;
}

void MyMemory_MarkAllExistingAllocationsAsStatic()
{
    return;

    // not ideal, but called from game code's WinMain.cpp ATM
    // If any code(in this case the bullet profiler clock btQuickProf.cpp) allocated memory in a static class instance,
    //    this will remove it from the allocation list that ValidateAllocations() checks on shutdown.
    // Turned off bullet profiling instead(#define BT_NO_PROFILE 1 in btQuickProf.cpp), but situation still possible

    // Used again in wxWidgets build, event tables (BEGIN_EVENT_TABLE...END_EVENT_TABLE) are statically allocated

    CPPListNode* pNode;

    for( pNode = g_pAllocationList->m_Allocations.HeadNode.Next; pNode && pNode->Next; )
    {
        CPPListNode* pNodeToMove = pNode;
        pNode = pNode->GetNext();

        MemObject* obj = (MemObject*)pNode;
        if( obj )
            LOGInfo( LOGTag, "Moving memory allocation to StaticallyAllocatedRam cpplist. %d bytes\n", obj->m_size );

        g_StaticallyAllocatedRam.MoveTail( pNodeToMove );
    }
}

void operator delete(void* m, char* file, unsigned long line)
{
    delete (char*)m;
}

void operator delete[](void* m, char* file, unsigned long line)
{
    delete (char*)m;
}

void* operator new(size_t size, char* file, unsigned long line)
{
    MyAssert( size > 0 );

    // grab the mutex, for g_pAllocationList and other tracking globals
    pthread_mutex_lock( &g_AllocationMutex );

    if( g_pAllocationList == 0 )
    {
        g_pAllocationList = (AllocationList*)1;
        pthread_mutex_unlock( &g_AllocationMutex );
        g_pAllocationList = new AllocationList;
        pthread_mutex_lock( &g_AllocationMutex );
    }
    if( g_pAllocationList == (AllocationList*)1 )
        g_pAllocationList = 0;

    MyAssert( file != 0 );
    //if( g_AllocatedRamCount == 83240 )
    //    int bp = 1;
    //if( file == 0 && size == 380 )
    //    int bp = 1;

    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_size = size;
    mo->m_type = newtype_reg;
    mo->m_file = file;
    mo->m_line = line;
    mo->m_allocationcount = g_AllocatedRamCount;
    if( g_pAllocationList == 0 )
    {
        MyAssert( g_AllocatedRamCount == 0 );
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = 0;
        mo->Prev = 0;
    }
    else
        g_pAllocationList->m_Allocations.AddTail( mo );

    if( mo->m_file == 0 )
    {
        //MyAssert( false );
        //LOGInfo( LOGTag, "Allocating ram without using MyNew\n" );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    g_TotalAllocatedRam += (int)size;
    g_AllocatedRamCount++;
    g_ActiveAllocatedRamCount++;

    pthread_mutex_unlock( &g_AllocationMutex );

    //MyMemory_GetNumberOfBytesAllocated();

    return ((char*)mo) + sizeof(MemObject); //mo + sizeof(MemObject);
}

void* operator new[](size_t size, char* file, unsigned long line)
{
    MyAssert( size > 0 );

    // grab the mutex, for g_pAllocationList and other tracking globals
    pthread_mutex_lock( &g_AllocationMutex );

    if( g_pAllocationList == 0 )
    {
        g_pAllocationList = (AllocationList*)1;
        pthread_mutex_unlock( &g_AllocationMutex );
        g_pAllocationList = new AllocationList;
        pthread_mutex_lock( &g_AllocationMutex );
    }
    if( g_pAllocationList == (AllocationList*)1 )
        g_pAllocationList = 0;

    //if( g_AllocatedRamCount == 83240 )
    //    int bp = 1;

    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_size = size;
    mo->m_type = newtype_array;
    mo->m_file = file;
    mo->m_line = line;
    mo->m_allocationcount = g_AllocatedRamCount;
    if( g_pAllocationList == 0 )
    {
        MyAssert( g_AllocatedRamCount == 0 );
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = 0;
        mo->Prev = 0;
    }
    else
        g_pAllocationList->m_Allocations.AddTail( mo );

    if( mo->m_file == 0 )
    {
        //MyAssert( false );
        //LOGInfo( LOGTag, "Allocating ram without using MyNew\n" );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC ARRAY: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    g_TotalAllocatedRam += (int)size;
    g_AllocatedRamCount++;
    g_ActiveAllocatedRamCount++;

    pthread_mutex_unlock( &g_AllocationMutex );

    //MyMemory_GetNumberOfBytesAllocated();

    return ((char*)mo) + sizeof(MemObject); //mo + sizeof(MemObject);
}

void* operator new(size_t size)
{
    MyAssert( size > 0 );

    // grab the mutex, for g_pAllocationList and other tracking globals
    pthread_mutex_lock( &g_AllocationMutex );

    if( g_pAllocationList == 0 )
    {
        g_pAllocationList = (AllocationList*)1;
        pthread_mutex_unlock( &g_AllocationMutex );
        g_pAllocationList = new AllocationList;
        pthread_mutex_lock( &g_AllocationMutex );
    }
    if( g_pAllocationList == (AllocationList*)1 )
        g_pAllocationList = 0;

    //if( g_AllocatedRamCount == 83240 )
    //    int bp = 1;
    //if( size == 16 )
    //    int bp = 1;

    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_size = size;
    mo->m_type = newtype_reg;
    mo->m_file = 0;
    mo->m_line = 0;
    mo->m_allocationcount = g_AllocatedRamCount;
    if( g_pAllocationList == 0 )
    {
        MyAssert( g_AllocatedRamCount == 0 );
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = 0;
        mo->Prev = 0;
    }
    else
        g_pAllocationList->m_Allocations.AddTail( mo );

    if( mo->m_file == 0 )
    {
        //MyAssert( false );
        //LOGInfo( LOGTag, "Allocating ram without using MyNew\n" );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    g_TotalAllocatedRam += (int)size;
    g_AllocatedRamCount++;
    g_ActiveAllocatedRamCount++;

    pthread_mutex_unlock( &g_AllocationMutex );

    //MyMemory_GetNumberOfBytesAllocated();

    return ((char*)mo) + sizeof(MemObject); //mo + sizeof(MemObject);
}

void operator delete(void* m)
{
    if( m == 0 )
        return;

    // grab the mutex, for g_pAllocationList and other tracking globals
    pthread_mutex_lock( &g_AllocationMutex );

    MemObject* mo = (MemObject*)(((char*)m) - sizeof(MemObject));
    size_t size = mo->m_size;
    MyAssert( mo->m_type == newtype_reg );
    //if( mo->m_type == newtype_reg )
    {
        if( mo->m_allocationcount == 0 )
            LOGInfo( LOGTag, "deleting allocation list...\n" );
        else if( mo->Next == 0 )
            LOGInfo( LOGTag, "deleting object that wasn't added to list...\n" );
        else
            mo->Remove();
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "FREE: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, m, mo->m_file, mo->m_line );
#endif

    //memset(mo, 0, sizeof(mo));
    
    int thisallocationcount = mo->m_allocationcount;

    g_TotalAllocatedRam -= (int)size;
    g_ActiveAllocatedRamCount--;
    free(mo);

    pthread_mutex_unlock( &g_AllocationMutex );

    //MyMemory_GetNumberOfBytesAllocated();

    // will only work if the first allocation is also the last one freed... likely a static allocation.
    if( thisallocationcount == 1 )
    {
        AllocationList* pList = g_pAllocationList;
        g_pAllocationList = 0;
        delete pList;
    }
}

void* operator new[](size_t size)
{
    MyAssert( size > 0 );

    // grab the mutex, for g_pAllocationList and other tracking globals
    pthread_mutex_lock( &g_AllocationMutex );

    if( g_pAllocationList == 0 )
    {
        g_pAllocationList = (AllocationList*)1;
        pthread_mutex_unlock( &g_AllocationMutex );
        g_pAllocationList = new AllocationList;
        pthread_mutex_lock( &g_AllocationMutex );
    }
    if( g_pAllocationList == (AllocationList*)1 )
        g_pAllocationList = 0;

    //if( g_AllocatedRamCount == 83240 )
    //    int bp = 1;

    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_size = size;
    mo->m_type = newtype_array;
    mo->m_file = 0;
    mo->m_line = 0;
    mo->m_allocationcount = g_AllocatedRamCount;
    if( g_pAllocationList == 0 )
    {
        MyAssert( g_AllocatedRamCount == 0 );
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = 0;
        mo->Prev = 0;
    }
    else
        g_pAllocationList->m_Allocations.AddTail( mo );

    if( mo->m_file == 0 )
    {
        //MyAssert( false );
        //LOGInfo( LOGTag, "Allocating ram without using MyNew\n" );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC ARRAY: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    g_TotalAllocatedRam += (int)size;
    g_AllocatedRamCount++;
    g_ActiveAllocatedRamCount++;

    pthread_mutex_unlock( &g_AllocationMutex );

    //MyMemory_GetNumberOfBytesAllocated();

    return ((char*)mo) + sizeof(MemObject); //mo + sizeof(MemObject);
}

void operator delete[](void* m)
{
    if( m == 0 )
        return;

    // grab the mutex, for g_pAllocationList and other tracking globals
    pthread_mutex_lock( &g_AllocationMutex );

    MemObject* mo = (MemObject*)( ((char*)m) - sizeof(MemObject) );
    size_t size = mo->m_size;
    MyAssert( mo->m_type == newtype_array );
    //if( mo->m_type == newtype_array )
    {
        if( mo->m_allocationcount == 0 )
            LOGInfo( LOGTag, "deleting allocation list...\n" );
        else if( mo->Next == 0 )
            LOGInfo( LOGTag, "deleting object that wasn't added to list...\n" );
        else
            mo->Remove();
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "FREE ARRAY: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, m, mo->m_file, mo->m_line );
#endif

    int thisallocationcount = mo->m_allocationcount;

    g_TotalAllocatedRam -= (int)size;
    g_ActiveAllocatedRamCount--;
    free(mo);

    pthread_mutex_unlock( &g_AllocationMutex );

    //MyMemory_GetNumberOfBytesAllocated();

    // will only work if the first allocation is also the last one freed... likely a static allocation.
    if( thisallocationcount == 1 )
    {
        AllocationList* pList = g_pAllocationList;
        g_pAllocationList = 0;
        delete pList;
    }
}

#endif //MYFW_WINDOWS
