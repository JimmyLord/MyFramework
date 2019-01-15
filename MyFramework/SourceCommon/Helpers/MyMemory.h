//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyMemory_H__
#define __MyMemory_H__

#include "../DataTypes/MyTypes.h"

#define SAFE_DELETE(x) { delete x; x = nullptr; }
#define SAFE_DELETE_ARRAY(x) { delete[] x; x = nullptr; }
#define SAFE_RELEASE(x) { if( x ) x->Release(); x = nullptr; }

#if !MYFW_WINDOWS

#define MyNew new

#elif MYFW_WINDOWS

enum NewTypes
{
    NewType_NotSet = 0,
    NewType_Regular = 1,
    NewType_Array = 2,
};

#define MyNew new(__FILE__, __LINE__)

class MemObject : public TCPPListNode<MemObject*>
{
public:
    size_t m_Size;
    unsigned long m_Line;
    const char* m_File;
    NewTypes m_Type;
    int m_AllocationCount;

    MemObject() :
        m_Size( 0 ),
        m_Line( 0 ),
        m_File( nullptr ),
        m_Type( NewType_NotSet ),
        m_AllocationCount( 0 )
    {
    }
};

class AllocationList;

MemObject* MyMemory_GetFirstMemObject();
void MyMemory_ValidateAllocations(AllocationList* pList, bool assertOnAnyAllocation);
size_t MyMemory_GetNumberOfBytesAllocated();
uint32 MyMemory_GetNumberOfMemoryAllocations();
uint32 MyMemory_GetNumberOfActiveMemoryAllocations();
void MyMemory_MarkAllExistingAllocationsAsStatic();

void* operator new(size_t size, const char* file, unsigned long line);
void* operator new[](size_t size, const char* file, unsigned long line);
void operator delete(void* ptr, const char* file, unsigned long line);
void operator delete[](void* ptr, const char* file, unsigned long line);

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr);
void operator delete[](void* ptr);

#endif //MYFW_WINDOWS

#endif //__MyMemory_H__
