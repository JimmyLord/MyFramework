//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __StackAllocator_H__
#define __StackAllocator_H__

class MyStackAllocator
{
public:
    typedef char* MyStackPointer;

protected:
    char* m_pMemory;
    char* m_pCurrentPointer;

    unsigned int m_SizeInBytes;

public:
    MyStackAllocator();
    ~MyStackAllocator();

    void Initialize(unsigned int sizeinbytes);
    void Cleanup();

    MyStackPointer GetCurrentLocation();
    void RewindStack(MyStackPointer ptr);
    void Clear();

    void* AllocateBlock(unsigned int byteswanted, MyStackPointer* pStartLocation = 0);

    unsigned int GetBytesUsed();
};

#endif //__StackAllocator_H__
