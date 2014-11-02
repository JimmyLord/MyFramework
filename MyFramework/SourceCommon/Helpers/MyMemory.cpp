//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include <assert.h>

#if MYFW_WINDOWS

#define MEMORY_ShowDebugInfo   0

CPPListHead AllocatedRam;
int TotalAllocatedRam = 0;

void ValidateAllocations(bool AssertOnAnyAllocation)
{
#if MYFW_WINDOWS && _DEBUG
    CPPListNode* node;
    for( node = AllocatedRam.HeadNode.Next; node->Next; node = node->Next )
    {
        MemObject* obj = (MemObject*)node;
        assert( obj->m_size < 200000 );
        assert( obj->m_type < 3 );
        assert( obj->m_line < 2500 );
        assert( obj->Next != NULL );
        assert( obj->Prev != NULL );

        if( AssertOnAnyAllocation )
            assert( false );
        LOGInfo( LOGTag, "%s(%d): Memory unreleased.\n", obj->m_file, obj->m_line );
    }
#endif
}

int GetMemoryUsageCount()
{
    unsigned int count = 0;

#if MYFW_WINDOWS && _DEBUG
    CPPListNode* node;
    for( node = AllocatedRam.HeadNode.Next; node->Next; node = node->Next )
    {
        MemObject* obj = (MemObject*)node;
        count += obj->m_size;
    }

    LOGInfo( LOGTag, "Memory used %d\n", count );
#endif

    return count;
}

void operator delete(void* m, char* file, unsigned long line)
{
    delete((char*)m);
}

void operator delete[](void* m, char* file, unsigned long line)
{
    delete((char*)m);
}

void* operator new(size_t size, char* file, unsigned long line)
{
    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_size = size;
    mo->m_type = newtype_reg;
    mo->m_file = file;
    mo->m_line = line;
    if( AllocatedRam.HeadNode.Next == 0 )
    {
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = 0;
        mo->Prev = 0;
    }
    else
        AllocatedRam.AddHead(mo);

    if( mo->m_file == 0 )
    {
        //assert( false );
        LOGInfo( LOGTag, "Allocating ram without using MyNew\n" );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    TotalAllocatedRam += (int)size;
    return ((char*)mo) + sizeof(MemObject); //mo + sizeof(MemObject);
}

void* operator new[](size_t size, char* file, unsigned long line)
{
    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_size = size;
    mo->m_type = newtype_array;
    mo->m_file = file;
    mo->m_line = line;
    if( AllocatedRam.HeadNode.Next == 0 )
    {
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = 0;
        mo->Prev = 0;
    }
    else
        AllocatedRam.AddHead(mo);

    if( mo->m_file == 0 )
    {
        //assert( false );
        LOGInfo( LOGTag, "Allocating ram without using MyNew\n" );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC ARRAY: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    TotalAllocatedRam += (int)size;
    return ((char*)mo) + sizeof(MemObject); //mo + sizeof(MemObject);
}

void* operator new(size_t size)
{
    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_size = size;
    mo->m_type = newtype_reg;
    mo->m_file = 0;
    mo->m_line = 0;
    if( AllocatedRam.HeadNode.Next == 0 )
    {
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = 0;
        mo->Prev = 0;
    }
    else
        AllocatedRam.AddHead(mo);

    if( mo->m_file == 0 )
    {
        //assert( false );
        LOGInfo( LOGTag, "Allocating ram without using MyNew\n" );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    TotalAllocatedRam += (int)size;
    return ((char*)mo) + sizeof(MemObject); //mo + sizeof(MemObject);
}

void operator delete(void* m)
{
    if( m == 0 )
        return;

    MemObject* mo = (MemObject*)(((char*)m) - sizeof(MemObject));
    size_t size = mo->m_size;
    assert( mo->m_type == newtype_reg );
    //if( mo->m_type == newtype_reg )
    {
        if( mo->Next == 0 )
            LOGInfo( LOGTag, "deleting object that wasn't added to list...\n" );
        else
            mo->Remove();
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "FREE: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, m, mo->m_file, mo->m_line );
#endif

    //memset(mo, 0, sizeof(mo));
    TotalAllocatedRam -= (int)size;
    free(mo);
}

void* operator new[](size_t size)
{
    MemObject* mo = (MemObject*)malloc( size + sizeof(MemObject) );
    mo->m_size = size;
    mo->m_type = newtype_array;
    mo->m_file = 0;
    mo->m_line = 0;
    if( AllocatedRam.HeadNode.Next == 0 )
    {
        LOGInfo( LOGTag, "AllocatedRam table not initialized...\n" );
        mo->Next = 0;
        mo->Prev = 0;
    }
    else
        AllocatedRam.AddHead(mo);

    if( mo->m_file == 0 )
    {
        //assert( false );
        LOGInfo( LOGTag, "Allocating ram without using MyNew\n" );
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "ALLOC ARRAY: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, ((char*)mo) + sizeof(MemObject), mo->m_file, mo->m_line );
#endif

    TotalAllocatedRam += (int)size;
    return ((char*)mo) + sizeof(MemObject); //mo + sizeof(MemObject);
}

void operator delete[](void* m)
{
    if( m == 0 )
        return;

    MemObject* mo = (MemObject*)( ((char*)m) - sizeof(MemObject) );
    size_t size = mo->m_size;
    assert( mo->m_type == newtype_array );
    //if( mo->m_type == newtype_array )
    {
        if( mo->Next == 0 )
            LOGInfo( LOGTag, "deleting object that wasn't added to list...\n" );
        else
            mo->Remove();
    }

#if MEMORY_ShowDebugInfo
    LOGInfo( LOGTag, "FREE ARRAY: %d, 0x%p, 0x%p, %s, %d\n", (int)mo->m_size, mo, m, mo->m_file, mo->m_line );
#endif

    TotalAllocatedRam -= (int)size;
    free(mo);
}

#endif //MYFW_WINDOWS
