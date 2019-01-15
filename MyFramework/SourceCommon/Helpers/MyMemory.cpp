//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#if MYFW_WINDOWS

#define MEMORY_ShowDebugInfo    0
#define CountToAssertOn         UINT_MAX    // Set this to assert when 'new' has been called this many times.

class AllocationList;

TCPPListHead<MemObject*> g_StaticallyAllocatedRam;
AllocationList* g_pAllocationList = nullptr;
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
    TCPPListHead<MemObject*> m_Allocations;

    AllocationList()
    {
        int bp = 1;
    }

    ~AllocationList()
    {
        MyMemory_ValidateAllocations( this, true );
    }
};

MemObject* MyMemory_GetFirstMemObject()
{
    return g_pAllocationList->m_Allocations.GetHead();
}

void MyMemory_ValidateAllocations(AllocationList* pList, bool assertOnAnyAllocation)
{
#if _DEBUG
    // Grab the mutex, for g_pAllocationList and other tracking globals.
    pthread_mutex_lock( &g_AllocationMutex );

    LOGInfo( LOGTag, "Start dumping unfreed memory allocations.\n" );
    LOGInfo( LOGTag, "\\/\\/\\/\\/\\/\\/ Start of memory leak dump \\/\\/\\/\\/\\/\\/ \n" );
    for( MemObject* pMemObject = pList->m_Allocations.GetHead(); pMemObject; pMemObject = pMemObject->GetNext() )
    {
        MyAssert( pMemObject->m_Size < 200000 );
        MyAssert( pMemObject->m_Type < 3 );
        MyAssert( pMemObject->m_Line < 2500 );
        MyAssert( pMemObject->Next != nullptr );
        MyAssert( pMemObject->Prev != nullptr );

        // Pointer to allocation not freed: ((char*)pMemObject)+sizeof(MemObject)

        LOGInfo( LOGTag, "%s(%d): %d bytes unreleased. Count:(%d) %s\n", pMemObject->m_File, pMemObject->m_Line, pMemObject->m_Size, pMemObject->m_AllocationCount, pMemObject->m_Type == NewType_Regular ? "" : "Array Allocation" );
    }
    LOGInfo( LOGTag, "/\\/\\/\\/\\/\\/\\ End of memory leak dump /\\/\\/\\/\\/\\/\\ \n" );

    if( assertOnAnyAllocation && pList->m_Allocations.GetHead() != nullptr )
        MyAssert( false );

    pthread_mutex_unlock( &g_AllocationMutex );
#endif //_DEBUG
}

size_t MyMemory_GetNumberOfBytesAllocated()
{
    if( g_pAllocationList == nullptr )
        return 0;

#if _DEBUG
    // Grab the mutex, for g_pAllocationList and other tracking globals.
    pthread_mutex_lock( &g_AllocationMutex );

    // Since the list itself isn't in the list, start with it's size.
    size_t count = sizeof( AllocationList );

    MemObject* pMemObject;
    for( pMemObject = g_pAllocationList->m_Allocations.GetHead(); pMemObject; pMemObject = pMemObject->GetNext() )
    {
        count += pMemObject->m_Size;
    }

    for( pMemObject = g_StaticallyAllocatedRam.GetHead(); pMemObject; pMemObject = pMemObject->GetNext() )
    {
        count += pMemObject->m_Size;
    }

    MyAssert( count == g_TotalAllocatedRam );

    pthread_mutex_unlock( &g_AllocationMutex );

    return count;
#else
    return 0;
#endif //_DEBUG
}

uint32 MyMemory_GetNumberOfMemoryAllocations()
{
    return g_AllocatedRamCount;
}

uint32 MyMemory_GetNumberOfActiveMemoryAllocations()
{
    return g_ActiveAllocatedRamCount;
}

void MyMemory_MarkAllExistingAllocationsAsStatic()
{
    return;

    // Not ideal, but called from game code's WinMain.cpp ATM.
    // If any code(in this case the bullet profiler clock btQuickProf.cpp) allocated memory in a static class instance,
    //    this will remove it from the allocation list that ValidateAllocations() checks on shutdown.
    // Turned off bullet profiling instead(#define BT_NO_PROFILE 1 in btQuickProf.cpp), but situation still possible.

    // Used again in wxWidgets build, event tables (BEGIN_EVENT_TABLE...END_EVENT_TABLE) are statically allocated.

    //for( MemObject* pMemObject = g_pAllocationList->m_Allocations.GetHead(); pMemObject; )
    //{
    //    MemObject* pNextMemObject = pMemObject->GetNext();

    //    LOGInfo( LOGTag, "Moving memory allocation to StaticallyAllocatedRam cpplist. %d bytes\n", pMemObject->m_Size );
    //    g_StaticallyAllocatedRam.MoveTail( pMemObject );

    //    pMemObject = pNextMemObject;
    //}
}

void* ActualNew(NewTypes type, size_t size, const char* file, unsigned long line)
{
    MyAssert( CountToAssertOn == UINT_MAX || g_AllocatedRamCount != CountToAssertOn );
    MyAssert( size > 0 );
    MyAssert( file != nullptr );

    // Grab the mutex, for g_pAllocationList and other tracking globals.
    pthread_mutex_lock( &g_AllocationMutex );

    if( g_pAllocationList == nullptr )
    {
        g_pAllocationList = (AllocationList*)1;
        pthread_mutex_unlock( &g_AllocationMutex );
        g_pAllocationList = new AllocationList;
        pthread_mutex_lock( &g_AllocationMutex );
    }
    if( g_pAllocationList == (AllocationList*)1 )
    {
        g_pAllocationList = nullptr;
    }

    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_Size = size;
    mo->m_Type = type;
    mo->m_File = file;
    mo->m_Line = line;
    mo->m_AllocationCount = g_AllocatedRamCount;
    if( g_pAllocationList == nullptr )
    {
        MyAssert( g_AllocatedRamCount == 0 );
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = nullptr;
        mo->Prev = nullptr;
    }
    else
    {
        g_pAllocationList->m_Allocations.AddTail( mo );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC %s: %d, 0x%p, 0x%p, %s, %d\n", type == NewType_Regular ? "" : "ARRAY", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    g_TotalAllocatedRam += (int)size;
    g_AllocatedRamCount++;
    g_ActiveAllocatedRamCount++;

    pthread_mutex_unlock( &g_AllocationMutex );

    return ((char*)mo) + sizeof(MemObject);
}

void ActualDelete(NewTypes type, void* ptr)
{
    if( ptr == nullptr )
        return;

    // Grab the mutex, for g_pAllocationList and other tracking globals.
    pthread_mutex_lock( &g_AllocationMutex );

    MemObject* pMemObject = (MemObject*)(((char*)ptr) - sizeof(MemObject));
    size_t size = pMemObject->m_Size;
    MyAssert( pMemObject->m_Type == type );

    if( pMemObject->m_AllocationCount == 0 )
        LOGInfo( LOGTag, "Deleting allocation list...\n" );
    else if( pMemObject->Next == nullptr )
        LOGInfo( LOGTag, "Deleting object that wasn't added to list...\n" );
    else
        pMemObject->Remove();

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "FREE %s: %d, 0x%p, 0x%p, %s, %d\n", type == NewType_Regular ? "" : "ARRAY", (int)mo->m_size, mo, m, mo->m_file, mo->m_line );
#endif
    
    int thisAllocationCount = pMemObject->m_AllocationCount;

    g_TotalAllocatedRam -= (int)size;
    g_ActiveAllocatedRamCount--;
    free( pMemObject );

    pthread_mutex_unlock( &g_AllocationMutex );

    // Will only work if the first allocation is also the last one freed... likely a static allocation.
    if( thisAllocationCount == 1 )
    {
        AllocationList* pList = g_pAllocationList;
        g_pAllocationList = nullptr;
        delete pList;
    }
}

void* operator new(size_t size, const char* file, unsigned long line)
{
    return ActualNew( NewType_Regular, size, file, line );
}

void* operator new[](size_t size, const char* file, unsigned long line)
{
    return ActualNew( NewType_Array, size, file, line );
}

void operator delete(void* ptr, const char* file, unsigned long line)
{
    ActualDelete( NewType_Regular, ptr );
}

void operator delete[](void* ptr, const char* file, unsigned long line)
{
    ActualDelete( NewType_Array, ptr );
}

void* operator new(size_t size)
{
    return ActualNew( NewType_Regular, size, "File not defined", 0 );
}

void* operator new[](size_t size)
{
    return ActualNew( NewType_Array, size, "File not defined", 0 );
}

void operator delete(void* ptr)
{
    ActualDelete( NewType_Regular, ptr );
}

void operator delete[](void* ptr)
{
    ActualDelete( NewType_Array, ptr );
}

#endif //MYFW_WINDOWS
