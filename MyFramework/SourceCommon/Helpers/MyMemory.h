//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MYMEMORY_H__
#define __MYMEMORY_H__

#define SAFE_DELETE(x) { delete x; x=0; }
#define SAFE_DELETE_ARRAY(x) { delete[] x; x=0; }
#define SAFE_RELEASE(x) { if(x) x->Release(); x=0; }

#if !MYFW_WINDOWS

#define MyNew new

#elif MYFW_WINDOWS

#define MyNew               new(__FILE__, __LINE__)
//#define MyNewArray(x,y)      new(__FILE__, __LINE__) x[y]

#define newtype_reg         1
#define newtype_array       2

class MemObject : public CPPListNode
{
public:
    size_t m_size;
    unsigned long m_line;
    char* m_file;
    int m_type;
    int m_allocationcount;

    MemObject() :
        m_size(0),
        m_line(0),
        m_file(0),
        m_type(0),
        m_allocationcount(0)
    {
    }
};

class AllocationList;

void MyMemory_ValidateAllocations(AllocationList* pList, bool AssertOnAnyAllocation);
unsigned int MyMemory_GetNumberOfBytesAllocated();
unsigned int MyMemory_GetNumberOfMemoryAllocations();
void MyMemory_MarkAllExistingAllocationsAsStatic();

void* operator new(size_t size, char* file, unsigned long line);
void* operator new[](size_t size, char* file, unsigned long line);
void operator delete(void* m, char* file, unsigned long line);
void operator delete[](void* m, char* file, unsigned long line);

void* operator new(size_t size);
void operator delete(void* m);
void* operator new[](size_t size);
void operator delete[](void* m);

#endif //MYFW_WINDOWS

#endif //__MYMEMORY_H__
